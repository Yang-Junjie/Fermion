namespace Fermion
{
	public class Input
	{
		public static bool IsKeyDown(KeyCode keycode)
		{
			return InternalCalls.Input_IsKeyDown(keycode);
		}

		public static void SetCursorMode(CursorMode mode)
		{
			InternalCalls.Input_SetCursorMode(mode);
		}

		public static void SetMousePosition(float x, float y)
		{
			InternalCalls.Input_SetMousePosition(x, y);
		}

		public static void SetMousePosition(Vector2 position)
		{
			InternalCalls.Input_SetMousePosition(position.X, position.Y);
		}

		public static Vector2 GetMousePosition()
		{
			InternalCalls.Input_GetMousePosition(out float x, out float y);
			return new Vector2(x, y);
		}
	}
}
