// source/java/Frustum.cpp
// Adaption of Frustum.java from: Decompiled 'c0.0.11a.jar'

#include <nds.h>
#include "Frustum.hpp"

Frustum Frustum::instance;

Frustum& Frustum::getFrustum() {
    instance.calculateFrustum();
    return instance;
}

void Frustum::calculateFrustum() {
    int rawProj[16];
    int rawModl[16];

    glGetFixed(GL_GET_MATRIX_PROJECTION, rawProj);
    glGetFixed(GL_GET_MATRIX_POSITION, rawModl);

    for (int i = 0; i < 16; ++i) {
        proj[i] = f32tofloat(rawProj[i]);
        modl[i] = f32tofloat(rawModl[i]);
    }

    clip[0]  = modl[0] * proj[0] + modl[1] * proj[4] + modl[2] * proj[8]  + modl[3] * proj[12];
    clip[1]  = modl[0] * proj[1] + modl[1] * proj[5] + modl[2] * proj[9]  + modl[3] * proj[13];
    clip[2]  = modl[0] * proj[2] + modl[1] * proj[6] + modl[2] * proj[10] + modl[3] * proj[14];
    clip[3]  = modl[0] * proj[3] + modl[1] * proj[7] + modl[2] * proj[11] + modl[3] * proj[15];

    clip[4]  = modl[4] * proj[0] + modl[5] * proj[4] + modl[6] * proj[8]  + modl[7] * proj[12];
    clip[5]  = modl[4] * proj[1] + modl[5] * proj[5] + modl[6] * proj[9]  + modl[7] * proj[13];
    clip[6]  = modl[4] * proj[2] + modl[5] * proj[6] + modl[6] * proj[10] + modl[7] * proj[14];
    clip[7]  = modl[4] * proj[3] + modl[5] * proj[7] + modl[6] * proj[11] + modl[7] * proj[15];

    clip[8]  = modl[8] * proj[0] + modl[9] * proj[4] + modl[10] * proj[8]  + modl[11] * proj[12];
    clip[9]  = modl[8] * proj[1] + modl[9] * proj[5] + modl[10] * proj[9]  + modl[11] * proj[13];
    clip[10] = modl[8] * proj[2] + modl[9] * proj[6] + modl[10] * proj[10] + modl[11] * proj[14];
    clip[11] = modl[8] * proj[3] + modl[9] * proj[7] + modl[10] * proj[11] + modl[11] * proj[15];

    clip[12] = modl[12] * proj[0] + modl[13] * proj[4] + modl[14] * proj[8]  + modl[15] * proj[12];
    clip[13] = modl[12] * proj[1] + modl[13] * proj[5] + modl[14] * proj[9]  + modl[15] * proj[13];
    clip[14] = modl[12] * proj[2] + modl[13] * proj[6] + modl[14] * proj[10] + modl[15] * proj[14];
    clip[15] = modl[12] * proj[3] + modl[13] * proj[7] + modl[14] * proj[11] + modl[15] * proj[15];

    m_Frustum[RIGHT][A]  = clip[3]  - clip[0];
    m_Frustum[RIGHT][B]  = clip[7]  - clip[4];
    m_Frustum[RIGHT][C]  = clip[11] - clip[8];
    m_Frustum[RIGHT][D]  = clip[15] - clip[12];

    m_Frustum[LEFT][A]   = clip[3]  + clip[0];
    m_Frustum[LEFT][B]   = clip[7]  + clip[4];
    m_Frustum[LEFT][C]   = clip[11] + clip[8];
    m_Frustum[LEFT][D]   = clip[15] + clip[12];

    m_Frustum[BOTTOM][A] = clip[3]  + clip[1];
    m_Frustum[BOTTOM][B] = clip[7]  + clip[5];
    m_Frustum[BOTTOM][C] = clip[11] + clip[9];
    m_Frustum[BOTTOM][D] = clip[15] + clip[13];

    m_Frustum[TOP][A]    = clip[3]  - clip[1];
    m_Frustum[TOP][B]    = clip[7]  - clip[5];
    m_Frustum[TOP][C]    = clip[11] - clip[9];
    m_Frustum[TOP][D]    = clip[15] - clip[13];

    m_Frustum[BACK][A]   = clip[3]  - clip[2];
    m_Frustum[BACK][B]   = clip[7]  - clip[6];
    m_Frustum[BACK][C]   = clip[11] - clip[10];
    m_Frustum[BACK][D]   = clip[15] - clip[14];

    m_Frustum[FRONT][A]  = clip[3]  + clip[2];
    m_Frustum[FRONT][B]  = clip[7]  + clip[6];
    m_Frustum[FRONT][C]  = clip[11] + clip[10];
    m_Frustum[FRONT][D]  = clip[15] + clip[14];
}

bool Frustum::cubeInFrustum(float x1, float y1, float z1, float x2, float y2, float z2) {
    for (int i = 0; i < 6; ++i) {
        const float a = m_Frustum[i][A];
        const float b = m_Frustum[i][B];
        const float c = m_Frustum[i][C];
        const float d = m_Frustum[i][D];

        if (a * x1 + b * y1 + c * z1 + d <= 0.0f &&
            a * x2 + b * y1 + c * z1 + d <= 0.0f &&
            a * x1 + b * y2 + c * z1 + d <= 0.0f &&
            a * x2 + b * y2 + c * z1 + d <= 0.0f &&
            a * x1 + b * y1 + c * z2 + d <= 0.0f &&
            a * x2 + b * y1 + c * z2 + d <= 0.0f &&
            a * x1 + b * y2 + c * z2 + d <= 0.0f &&
            a * x2 + b * y2 + c * z2 + d <= 0.0f) {
            return false;
        }
    }
    return true;
}
