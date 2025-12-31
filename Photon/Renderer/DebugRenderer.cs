namespace Fermion
{
    public static class DebugRenderer
    {
        public static unsafe void DrawLine(Vector3 p0, Vector3 p1, Vector4 color)
        {
            InternalCalls.DebugRenderer_DrawLine(&p0, &p1, &color);
        }
        public static void SetLineWidth(float width)
        {
            InternalCalls.DebugRenderer_SetLineWidth(width);
        }
        public static unsafe void DrawQuadBillboard(Vector3 position, Vector2 scale, Vector4 color)
        {
            InternalCalls.DebugRenderer_DrawQuadBillboard(&position, &scale, &color);
        }
    }
}
