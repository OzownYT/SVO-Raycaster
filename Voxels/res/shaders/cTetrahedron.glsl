#version 450 core
layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f, binding = 0) uniform image2D outputImage;

uniform mat4 uCameraToWorld;
uniform mat4 uProjectionInverse;
uniform vec3 uLightPos;
uniform float uTime;
uniform samplerCube uSkybox;

#define MAX_STEPS 50
#define MAX_DIST 100.
#define SURF_DIST .01
#define TOTAL_BOUNCE 4
#define INF 1./0.;

const vec3 BACKGROUND = vec3(7, 44, 80);
const vec3 FRACTAL_COLOR = vec3(253, 120, 9);

struct Ray {
    vec3 Origin;
    vec3 Direction;
};

float GetDist(vec3 z) {
    float Scale = 2.;
    float Offset = 1.;
    float r;
    int n = 0;
    while (n < 8) {
       if(z.x+z.y<0) z.xy = -z.yx; // fold 1
       if(z.x+z.z<0) z.xz = -z.zx; // fold 2
       if(z.y+z.z<0) z.zy = -z.yz; // fold 3	
       z = z * Scale - Offset * (Scale-1.0);
       n++;
    }
    return (length(z) ) * pow(Scale, -float(n));
}

vec3 GetNormal(vec3 p) {
    float d = GetDist(p);
    vec2 e = vec2(.001, 0);
    
    vec3 n = d - vec3(
        GetDist(p + e.xyy) - GetDist(p - e.xyy),
        GetDist(p + e.yxy) - GetDist(p - e.yxy),
        GetDist(p + e.yyx) - GetDist(p - e.yyx)
    );

    return normalize(n);
}

float RayMarch(Ray ray) {
    float totalDistance = 0.;
    int steps;
    for (steps = 0; steps < MAX_STEPS; steps++) {
        vec3 p = ray.Origin + ray.Direction * totalDistance;
        float dist = GetDist(p);
        totalDistance += dist;
        
        if (dist < SURF_DIST) break;
    }
    return 1.0 - float(steps) / float(MAX_STEPS);
}

// float AO(in vec3 p) {
//     vec3 n = GetNormal(p);
//     float e = 0.1;
//     float res = 0.0;

//     for (int i = 1; i <= 5; ++i) {
//         float d = e * float(i);
//         res += 1.0 - (d - GetDist(p + d * n));
//     }
     
//     res /= 5.;

//     return res;
// }

vec3 Render(inout Ray ray) {
    vec3 pixel = BACKGROUND;
    
    float d = RayMarch(ray);
    
    if (d > SURF_DIST)
        pixel = vec3(FRACTAL_COLOR / 255.) * d;
    else pixel = BACKGROUND / 255.;
    return pixel;
}

void main() {
    Ray ray;
    
    ivec2 coords = ivec2(gl_GlobalInvocationID.xy); // Current pixel position in window space
    ivec2 dims = imageSize(outputImage); // Window size
    vec2 uv = (coords - .5 * dims) / dims;

    ray.Origin = (uCameraToWorld * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    ray.Direction = normalize(uProjectionInverse * vec4(uv.x, uv.y, 1., 1.0)).xyz;
    ray.Direction = normalize((uCameraToWorld * vec4(ray.Direction, 0.0)).xyz);

    vec3 pixel = Render(ray);
    
    imageStore(outputImage, coords, vec4(pixel, 1.0));
}