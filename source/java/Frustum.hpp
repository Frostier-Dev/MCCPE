#ifndef FRUSTUM_HPP
#define FRUSTUM_HPP

class Frustum {
public:
    float m_Frustum[6][4];

    static constexpr int RIGHT  = 0;
    static constexpr int LEFT   = 1;
    static constexpr int BOTTOM = 2;
    static constexpr int TOP    = 3;
    static constexpr int BACK   = 4;
    static constexpr int FRONT  = 5;

    static constexpr int A = 0;
    static constexpr int B = 1;
    static constexpr int C = 2;
    static constexpr int D = 3;

    static Frustum& getFrustum();
    bool cubeInFrustum(float x1, float y1, float z1, float x2, float y2, float z2);

private:
    Frustum() = default;
    static Frustum instance;

    float proj[16];
    float modl[16];
    float clip[16];

    void calculateFrustum();
};

#endif