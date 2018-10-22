float4 QuaternionMultiply(float4 q1, float4 q2)
{
    return float4(
        (q2[3] * q1[0]) + (q2[0] * q1[3]) + (q2[1] * q1[2]) - (q2[2] * q1[1]),
        (q2[3] * q1[1]) - (q2[0] * q1[2]) + (q2[1] * q1[3]) + (q2[2] * q1[0]),
        (q2[3] * q1[2]) + (q2[0] * q1[1]) - (q2[1] * q1[0]) + (q2[2] * q1[3]),
        (q2[3] * q1[3]) - (q2[0] * q1[0]) - (q2[1] * q1[1]) - (q2[2] * q1[2]));
}

float4x4 DualQuaternionToMatrix(float4 quatReal, float4 quatDual)
{
    float x = quatReal.x;
    float y = quatReal.y;
    float z = quatReal.z;
    float w = quatReal.w;

    float4x4 m;
    m._11 = w * w + x * x - y * y - z * z;
    m._12 = 2 * x * y + 2 * w * z;
    m._13 = 2 * x * z - 2 * w * y;
    m._14 = 0;

    m._21 = 2 * x * y - 2 * w * z;
    m._22 = w * w + y * y - x * x - z * z;
    m._23 = 2 * y * z + 2 * w * x;
    m._24 = 0;

    m._31 = 2 * x * z + 2 * w * y;
    m._32 = 2 * y * z - 2 * w * x;
    m._33 = w * w + z * z - x * x - y * y;
    m._34 = 0;

    float4 conj = float4(-quatReal.x, -quatReal.y, -quatReal.z, quatReal.w);
    float4 t = QuaternionMultiply((quatDual * 2.0f), conj);
    m._41 = t.x;
    m._42 = t.y;
    m._43 = t.z;
    m._44 = 1;

    return m;
}