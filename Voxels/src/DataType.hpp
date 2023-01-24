#pragma once

struct Vector3f {
    union {
        struct {
            float x, y, z;
        };
        float data[3];
    };

    Vector3f() {}
    Vector3f(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3f(float n) : x(n), y(n), z(n) {}

    float &operator[](int index) {
        return data[index];
    }
    Vector3f operator+(Vector3f other) {
        return Vector3f{
            x + other.x,
            y + other.y,
            z + other.z,
        };
    }
    Vector3f operator-(Vector3f other) {
        return Vector3f{
            x - other.x,
            y - other.y,
            z - other.z,
        };
    }
    Vector3f operator*(float scalar) {
        return Vector3f{
            x * scalar,
            y * scalar,
            z * scalar,
        };
    }
    Vector3f operator/(float scalar) {
        return Vector3f{
            x / scalar,
            y / scalar,
            z / scalar,
        };
    }
    Vector3f operator*(Vector3f other) {
        return Vector3f{
            x * other.x,
            y * other.y,
            z * other.z,
        };
    }
};

struct Vector2f {
    union {
        struct {
            float x, y;
        };
        float data[2];
    };

    float &operator[](int index) {
        return data[index];
    }
};

struct Vector3i {
    union {
        struct {
            int x, y, z;
        };
        int data[3];
    };

    int &operator[](int index) {
        return data[index];
    }
};

struct Vector3u {
    union {
        struct {
            unsigned int x, y, z;
        };
        unsigned int data[3];
    };

    Vector3u(unsigned int x, unsigned int y, unsigned int z) : x(x), y(y), z(z) {}
    Vector3u(unsigned int n) : x(n), y(n), z(n) {}

    unsigned int &operator[](int index) {
        return data[index];
    }
    Vector3f operator*(float scalar) {
        return Vector3f{
            (float)x * scalar,
            (float)y * scalar,
            (float)z * scalar,
        };
    }
};

struct Color {
    float r, g, b, a;
};

struct VoxelInfo {
    Color color;
};
