#version 450 core
layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f, binding = 0) uniform image2D outputImage;

uniform mat4 uCameraToWorld;
uniform mat4 uProjectionInverse;
uniform vec3 uLightPos;
uniform float uTime;
uniform samplerCube uSkybox;

#define MAX_STEPS 100
#define MAX_DIST 100.
#define SURF_DIST .01
#define TOTAL_BOUNCE 8
#define INF 1./0.;

float GetDist(vec3 p) {
    vec4 s[3];
    s[0] = vec4(0, 1, -6, 1);
    s[1] = vec4(3, 1, -6, 1);
    s[2] = vec4(-3, 1, -6, 1);
    
    float sphereDist = INF;
    for (int i = 0; i < 3; i++) { 
        float dist = length(p - s[i].xyz) - s[i].w;
        if (dist < sphereDist) sphereDist = dist;
    }
    // float planeDist = p.y;
    
    return sphereDist;
}

float RayMarch(vec3 ro, vec3 rd) {
    float dO = 0.;
    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 p = ro + rd * dO;
        float dist = GetDist(p);
        dO += dist;
        
        if (dO > MAX_DIST || dist < SURF_DIST) break;
    }
    return dO;
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

float GetLight(vec3 p) {
    vec3 lightPos = vec3(0, 5, -6);
    lightPos.xz += vec2(sin(uTime), cos(uTime)) * 5;
    vec3 l = normalize(lightPos - p);
    vec3 n = GetNormal(p);

    float dif = clamp(dot(n, l), -1., 1.);
    // float d = RayMarch(p + n * SURF_DIST * 2., l);
    // if (d < length(lightPos - p)) dif *= .1;

    return dif;
}

vec3 Render(inout vec3 ro,  inout vec3 rd, inout vec3 ref) {
    vec3 pixel = texture(uSkybox, rd).rgb;
    
    float d = RayMarch(ro, rd);
    
    if (d < MAX_DIST) {
        ref = vec3(.9);
        vec3 p = ro + rd * d;
        float ambient = 0.2;
        float dif = GetLight(p);
        
        vec3 r = reflect(rd, GetNormal(p)); 
        pixel = vec3(dif * 0.1);

        ro = p + GetNormal(p) * SURF_DIST * 3;
        rd = r;
    } else {
        ref = vec3(.0);
    }
    return pixel;
}

void main() {
    vec3 pixel = vec3(0.2, 0.2, 0.2);

    ivec2 coords = ivec2(gl_GlobalInvocationID.xy); // Current pixel position in window space
    ivec2 dims = imageSize(outputImage); // Window size
    vec2 uv = (coords - .5 * dims) / dims;

    vec3 ro =  (uCameraToWorld * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vec3 rd = normalize(uProjectionInverse * vec4(uv.x, uv.y, 1., 1.0)).xyz;
    rd = normalize((uCameraToWorld * vec4(rd, 0.0)).xyz);

    vec3 ref = vec3(.0);
    vec3 energy = vec3(1.0);
    pixel = Render(ro, rd, ref);
    for (int i = 0; i < TOTAL_BOUNCE; i++) {
        vec3 bounce = Render(ro, rd, ref) * energy;
        pixel += bounce;
        energy *= ref;
    }

    pixel = pow(pixel, vec3(.4545));


    imageStore(outputImage, coords, vec4(pixel, 1.0));
}