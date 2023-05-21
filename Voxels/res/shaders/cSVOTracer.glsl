#version 450 core
layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f, binding = 0) uniform image2D outputImage;

#define MAX_DEPTH 12

uniform mat4 uCameraToWorld;
uniform mat4 uProjectionInverse;
uniform float uTime;
uniform int uDepth;
uniform int uRotate;
uniform uint uFar[256];

float mincomp(in vec3 p) { return min(p.x,min(p.y,p.z)); }
float maxcomp(in vec3 p) { return max(p.x, max(p.y, p.z)); }

#define INF 1./0.
#define EPSILON 0.005
#define SIZE 1024.
#define MAX_ITERATIONS 500

vec3 sunLight  = normalize( vec3(  0.4, 0.4,  0.48 ) );
vec3 sunColour = vec3(1.0, .9, .83);
vec3 rdInv;
uint far;

struct RayHit {
    float t;
    vec3 pos;
    uint depth;
    vec3 normal;
};

struct Node {
    uint childDesc;
    uint value;
};

layout(std430, binding = 3) buffer octree {
	uint descriptors[];
};

struct StackEntry {
    uint node, pIndex;
};

StackEntry octreeStack[MAX_DEPTH + 1];
int stackPtr = 0;
void stackPush(StackEntry e) { octreeStack[stackPtr++] = e; }
StackEntry stackPop() { return octreeStack[--stackPtr]; }

bool IsValid(uint parent, uint idx) {
    return ((parent & 0xFF) & (1 << idx)) > 0;
}

uint GetChild(uint parent, uint idx, inout uint pIndex) {
    uint shift = 0;
    uint valid = parent & 0xFF;
    for (int i = 0; i < 8; i++) {
         if ((valid & (1 << i)) > 0) {
            if (idx == i) break;
            shift++;
        }
    }
    if (((parent >> 16) & 1) > 0) 
        pIndex = uFar[parent >> 17] + shift + pIndex;   
    else
        pIndex = (parent >> 17) + shift + pIndex;
    return descriptors[pIndex];
}

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 CalculateT(vec3 ro, vec3 rd, vec3 p) {
    return (p - ro) * rdInv;
}

uint SelectChild(vec3 ro, vec3 rd, inout uvec3 positions, float size, float tmin) {
    vec3 p = ro + rd * tmin;
    uvec3 childPos = uvec3(round((p - positions * size) / size));
    positions = uvec3(
        (positions.x << 1) | childPos.x,
        (positions.y << 1) | childPos.y,
        (positions.z << 1) | childPos.z
    );
    uint idx = (childPos.x << 0) | (childPos.y << 1) | (childPos.z) << 2;
    return idx;
}

int CheckNewPos(vec3 rd, uvec3 old, uvec3 new) {
    if (sign(rd.x) > 0 && new.x < old.x) return 1;
    if (sign(rd.y) > 0 && new.y < old.y) return 2;
    if (sign(rd.z) > 0 && new.z < old.z) return 4;
    if (sign(rd.x) < 0 && new.x > old.x) return 1;
    if (sign(rd.y) < 0 && new.y > old.y) return 2;
    if (sign(rd.z) < 0 && new.z > old.z) return 4;
    return 0;
}

bool RayMarch(vec3 ro, vec3 rd, inout RayHit rh) {
    uvec3 positions = uvec3(0);
    rdInv = 1 / rd;
    far = 0;

    vec3 rSign = step(0, sign(rd));
    
    // Find which edges are the starting point of the ray intersection
    // If the ray is positive we will enter through the min bounds of the volume
    // If the ray is negative we will enter through the max bounds of the volume
    vec3 t0 = CalculateT(ro, rd, (vec3(1) - rSign) * SIZE);
    
    // Find which edges are the endning point of the ray intersection
    // If the ray is positive we will exit through the max bounds of the volume
    // If the ray is negative we will exit through the min bounds of the volume     
    vec3 t1 = CalculateT(ro, rd, rSign * SIZE);
    
    // We find the enter point by finding the biggest start point
    // We find the exit point by finding the smallest end point
    float tmin = maxcomp(t0);
    tmin = max(0.f, tmin);
    float tmax = mincomp(t1);

    float h = tmax;
    uint parent = descriptors[0];
    uint idx = SelectChild(ro, rd, positions, SIZE, tmin);
    int depth = 1;
    uvec3 pos = positions;
    uint pIndex = 0;

    for (int i = 0; i < MAX_ITERATIONS; i++) {
        if (tmin > tmax || tmax < 0) return false;
        float size = (1.f / exp2(depth)) * SIZE;

        vec3 tc = CalculateT(ro, rd, positions * size + (rSign * size));
        float tc_max = mincomp(tc);



        // Child is intersected and valid, either draw or go down the tree
        if (IsValid(parent, idx)) {
            if (depth == uDepth) {
                
                rh.t = tmin;
                rh.pos = ro + rd * tmin;
                rh.pos = rh.pos / SIZE;
                // rh.pos = vec3(rand((positions * SIZE).xy), rand((positions * SIZE).yz), rand((positions * SIZE).zx));
                vec3 tv = CalculateT(ro, rd, positions * size + ((vec3(1) - rSign) * size));
                vec3 s = sign(rd) - vec3(0.01);
                rh.normal = (tv.x > tv.y && tv.x > tv.z)
                            ? vec3(-1, 0, 0)
                            : (tv.y > tv.z ? vec3(0, -1, 0) : vec3(0, 0, -1));
                rh.normal *= -s;
                // rh.pos = vec3(float(i) / float(MAX_ITERATIONS));
                rh.depth = i;
                return true;
            }
            uint prevPIndex = pIndex;
            uint child = GetChild(parent, idx, pIndex);
                     

            h = tc_max;
            StackEntry e = {parent, prevPIndex};
            stackPush(e);
            parent = child;
            idx =  SelectChild(ro, rd, positions, size, tmin);
            pos = uvec3(
                positions.x & 1,
                positions.y & 1,
                positions.z & 1
            );
            depth++;
            continue;
        }


        // Child is not valid, either advance to next sibling or pop
        uvec3 oldPos = uvec3(
            positions.x & 1,
            positions.y & 1,
            positions.z & 1
        );
        pos = uvec3(
            uint(tc_max == tc.x) ^ positions.x & 1,
            uint(tc_max == tc.y) ^ positions.y & 1,
            uint(tc_max == tc.z) ^ positions.z & 1
        );

        positions = uvec3(
            (positions.x & 0xFFFE) | pos.x,
            (positions.y & 0xFFFE) | pos.y,
            (positions.z & 0xFFFE) | pos.z
        );
        int axis = CheckNewPos(rd, oldPos, pos);
        if (axis != 0) {
            if (stackPtr == 0) {
                rh.pos = vec3(.0f);
                return true;
            }
            positions = uvec3(
                positions.x >> 1,
                positions.y >> 1,
                positions.z >> 1
            );
            idx = 255;
            depth--;
            StackEntry e = stackPop();
            parent = e.node;
            pIndex = e.pIndex;
        } else {
            idx = pos.x + (2 * pos.y) + (4 * pos.z);
            tmin = tc_max;
        }
    }
}

vec3 GetSky(in vec3 rd)
{
	float sunAmount = max( dot( rd, sunLight), 0.0 );
	float v = pow(1.0-max(rd.y,0.0),5.)*.5;
	vec3 sky = vec3(v * sunColour.x * 0.4 + 0.18, v * sunColour.y * 0.4 + 0.22, v * sunColour.z * 0.4 +.4);
	// Wide glare effect...
	sky = sky + sunColour * pow(sunAmount, 6.5)*.32;
	// Actual sun...
	sky = sky+ sunColour * min(pow(sunAmount, 1150.0), .3)*.65;
	return sky;
}

vec4 PostEffects(vec4 rgb, vec2 uv)
{
	return vec4((1.0 - exp(-rgb * 6.0)) * 1.0024);
}

void main() {
    vec3 pixel = vec3(0.2, 0.2, 0.2);

    ivec2 coords = ivec2(gl_GlobalInvocationID.xy); // Current pixel position in window space
    ivec2 dims = imageSize(outputImage); // Window size
    vec2 uv = (coords - .5 * dims) / dims;

    vec3 ro =  (uCameraToWorld * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vec3 rd = normalize(uProjectionInverse * vec4(uv.x, uv.y, 1., 1.0)).xyz;
    rd = normalize((uCameraToWorld * vec4(rd, 0.0)).xyz);

    RayHit rh;
    if (RayMarch(ro, rd, rh))
        pixel = rh.pos ;
        // pixel = rh.pos * (0.2 + (max(dot(rh.normal, -sunLight), 0.0) * 0.5));
    else 
        pixel = GetSky(rd);
    
    
    pixel = PostEffects(vec4(pixel, 1.0), uv).xyz;

    imageStore(outputImage, coords, vec4(pixel, 1.0));
}