#version 450 core
layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f, binding = 0) uniform image2D outputImage;

#define INF 1./0.
#define EPSILON 0.005

layout(std430, binding = 3) readonly buffer shaderStorage {
    uint octreeBuffer[];
};

uniform mat4 uCameraToWorld;
uniform mat4 uProjectionInverse;
uniform sampler2D uNoiseTexture;
uniform sampler3D uVoxels;
uniform float uTime;

float mincomp(in vec3 p ) { return min(p.x,min(p.y,p.z));}

vec3 sunLight  = normalize( vec3(  0.4, 0.4,  0.48 ) );
vec4 sunColour = vec4(1.0, .9, .83, 1.0);
float specular = 0.0;
float ambient = 0.2;

struct RayHit {
    vec3 Position;
    float Distance;
    vec3 Normal;
};

struct Ray {
    vec3 Origin;
    vec3 Direction;
} ray;


float GetDist(vec3 p) {
    vec4 s = vec4(0, 1, -6, 1);
    
    float sphere = length(p - s.xyz) - s.w;
    float ground = p.y;
    // ground = p.y - texture(uNoiseTexture, p.xz * 0.05).r * 255.;

    return min(sphere, ground);
}

vec3 GetNormal(vec3 p) {
    float d = GetDist(p);
    vec2 e = vec2(.001, 0);
    
    vec3 n = d - vec3(
        GetDist(p - e.xyy),
        GetDist(p - e.yxy),
        GetDist(p - e.yyx)
    );

    return normalize(n);
}

vec4 RayMarch(in Ray ray, inout RayHit rayHit) {
    float depth = 0.0;
    int steps = 0;
    for (steps = 0; steps < 128; steps++) {
        vec3 p = ray.Origin + ray.Direction * depth;
        vec4 col = textureLod(uVoxels, p / 32, 0);
        // float dist = GetDist(ray.Origin + ray.Direction * depth);
        // depth += dist;
        if (col.a > 0) {
            rayHit.Position = p;
            rayHit.Distance = depth;
            rayHit.Normal = GetNormal(rayHit.Position);
            return col;
        }

        // Step returns 0 if ray direction is negative, otherwise it returns 1.
        vec3 delta = (step(0, ray.Direction) - fract(p)) / ray.Direction;
        depth += max(mincomp(delta), EPSILON);

        // if (depth > 100. || dist < 0.001) break;
    }

    return vec4(0);
}

vec4 GetLighting(vec3 p)
{
    vec3 normal = GetNormal(p);
	float diffuse = dot(sunLight,normal);

    // Ray r = { p + normal * 0.01 * 2., sunLight};
    // float d = RayMarch(r);
    // if (d < length(sunLight)) diffuse *= .1;

	return sunColour * max(diffuse, 0.0) + ambient;
}

vec4 GetSky(in vec3 rd)
{
	float sunAmount = max( dot( rd, sunLight), 0.0 );
	float v = pow(1.0-max(rd.y,0.0),5.)*.5;
	vec4 sky = vec4(v * sunColour.x * 0.4 + 0.18, v * sunColour.y * 0.4 + 0.22, v * sunColour.z * 0.4 +.4, 1.0);
	// Wide glare effect...
	sky = sky + sunColour * pow(sunAmount, 6.5)*.32;
	// Actual sun...
	sky = sky+ sunColour * min(pow(sunAmount, 1150.0), .3)*.65;
	return sky;
}

//--------------------------------------------------------------------------
// // Merge mountains into the sky background for correct disappearance...
// vec3 ApplyFog( in vec3  rgb, in float dis, in vec3 dir)
// {
// 	float fogAmount = exp(-dis* 0.00005);
// 	return mix(GetSky(dir), rgb, fogAmount );
// }


vec4 PostEffects(vec4 rgb, vec2 uv)
{
	return vec4((1.0 - exp(-rgb * 6.0)) * 1.0024);
}

void main() {
    vec4 color;
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dims = imageSize(outputImage);
    vec2 uv = (coords - .5 * dims) / dims;

    ray.Origin = (uCameraToWorld * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    ray.Direction = normalize(uProjectionInverse * vec4(uv, 1., 1.0)).xyz;
    ray.Direction = normalize((uCameraToWorld * vec4(ray.Direction, 0.0)).xyz);

    RayHit rHit = {vec3(0), 0, vec3(0)};

    color = RayMarch(ray, rHit);

    if (color.a > 0) {
        color = color * GetLighting(rHit.Position);
        // color = ApplyFog(color, depth, ray.Direction);
    }
    else color = GetSky(ray.Direction);

    color = PostEffects(color, uv);

    imageStore(outputImage, coords, color);
}