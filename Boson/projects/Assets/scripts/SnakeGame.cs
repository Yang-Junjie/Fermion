using System;
using System.Collections.Generic;
using Fermion;

namespace Photon
{
    public class SimpleRandom
    {
        private uint m_Seed;
        public SimpleRandom(uint seed) { m_Seed = seed; }
        public int Next(int maxValue)
        {
            m_Seed = (1103515245u * m_Seed + 12345u) & 0x7fffffffu;
            return (int)(m_Seed % (uint)maxValue);
        }
    }

    public class SnakeGame : Entity
    {
        // ========== 可配置参数 ==========
        public int GridWidth = 20;
        public int GridHeight = 20;
        public float CellSize = 1.0f;
        public float MoveInterval = 0.15f;

        // ========== 颜色配置 ==========
        private readonly Vector4 HeadColor = new Vector4(0.2f, 0.8f, 0.2f, 1.0f);
        private readonly Vector4 BodyColor = new Vector4(0.1f, 0.6f, 0.1f, 1.0f);
        private readonly Vector4 FoodColor = new Vector4(0.9f, 0.2f, 0.2f, 1.0f);
        private readonly Vector4 BorderColor = new Vector4(0.5f, 0.5f, 0.5f, 1.0f);
        private readonly Vector4 GameOverColor = new Vector4(0.8f, 0.1f, 0.1f, 1.0f);

        // ========== 游戏状态 ==========
        private enum Direction { Up, Down, Left, Right }

        private List<Vector2> m_SnakeBody = new List<Vector2>();
        private List<Entity> m_SnakeEntities = new List<Entity>();
        private Vector2 m_FoodPosition;
        private Entity m_FoodEntity;
        
        private Direction m_CurrentDirection = Direction.Right;
        private Queue<Direction> m_InputQueue = new Queue<Direction>(); 
        private const int MAX_INPUT_BUFFER = 2; 

        private float m_MoveTimer = 0.0f;
        private float m_CurrentMoveInterval;
        private bool m_GameOver = false;
        private int m_Score = 0;
        private SimpleRandom m_Random;
        private Entity m_ScoreEntity;
        private TextComponent m_ScoreText;

        // 按键状态记录
        private bool m_LastUpDown, m_LastDownDown, m_LastLeftDown, m_LastRightDown, m_LastRDown;

        // ========== 生命周期 ==========

        public void OnCreate()
        {
            float halfW = GridWidth * CellSize * 0.5f;
            float halfH = GridHeight * CellSize * 0.5f;

            m_ScoreEntity = Scene.CreateEntity("ScoreText");
            m_ScoreEntity.GetComponent<TransformComponent>().Translation =
                new Vector3(-halfW, halfH + CellSize * 0.5f, 0.0f);
            m_ScoreText = m_ScoreEntity.AddComponent<TextComponent>();

            CreateBorder();
            StartGame();
        }

        public void OnUpdate(float ts)
        {
            HandleInput();

            if (m_GameOver) return;

            m_MoveTimer += ts;
            if (m_MoveTimer >= m_CurrentMoveInterval)
            {
                m_MoveTimer -= m_CurrentMoveInterval; // 使用减法保持计时精度
                MoveSnake();
            }
        }

        // ========== 游戏初始化 ==========

        private void StartGame()
        {
            if (m_Random == null) m_Random = new SimpleRandom((uint)GetHashCode());

            ClearOldEntities();
            InitializeSnake();

            // 重置状态
            m_InputQueue.Clear();
            m_CurrentDirection = Direction.Right;
            m_MoveTimer = 0.0f;
            m_CurrentMoveInterval = MoveInterval;
            m_GameOver = false;
            m_Score = 0;
            UpdateScoreText();

            SpawnFood();
        }

        // ========== 输入处理逻辑 (优化核心) ==========

        private void HandleInput()
        {
            bool upDown = Input.IsKeyDown(KeyCode.Up) || Input.IsKeyDown(KeyCode.W);
            bool downDown = Input.IsKeyDown(KeyCode.Down) || Input.IsKeyDown(KeyCode.S);
            bool leftDown = Input.IsKeyDown(KeyCode.Left) || Input.IsKeyDown(KeyCode.A);
            bool rightDown = Input.IsKeyDown(KeyCode.Right) || Input.IsKeyDown(KeyCode.D);
            bool rDown = Input.IsKeyDown(KeyCode.R);

            // 转向输入检测
            if (upDown && !m_LastUpDown) TryQueueDirection(Direction.Up);
            else if (downDown && !m_LastDownDown) TryQueueDirection(Direction.Down);
            else if (leftDown && !m_LastLeftDown) TryQueueDirection(Direction.Left);
            else if (rightDown && !m_LastRightDown) TryQueueDirection(Direction.Right);

            if (rDown && !m_LastRDown) StartGame();

            m_LastUpDown = upDown; m_LastDownDown = downDown; m_LastLeftDown = leftDown;
            m_LastRightDown = rightDown; m_LastRDown = rDown;
        }

        private void TryQueueDirection(Direction dir)
        {
            if (m_InputQueue.Count >= MAX_INPUT_BUFFER) return;

            // 获取当前逻辑链条中最后一个预定方向
            Direction lastPending = m_InputQueue.Count > 0 ? 
                m_InputQueue.ToArray()[m_InputQueue.Count - 1] : m_CurrentDirection;

            // 过滤重复按键和相反方向
            if (dir != lastPending && !IsOpposite(dir, lastPending))
            {
                m_InputQueue.Enqueue(dir);
            }
        }

        private bool IsOpposite(Direction a, Direction b)
        {
            return (a == Direction.Up && b == Direction.Down) ||
                   (a == Direction.Down && b == Direction.Up) ||
                   (a == Direction.Left && b == Direction.Right) ||
                   (a == Direction.Right && b == Direction.Left);
        }

        // ========== 蛇移动逻辑 ==========

        private void MoveSnake()
        {
            if (m_SnakeEntities.Count == 0) return;

            // 从缓冲队列中取出下一个待执行方向
            if (m_InputQueue.Count > 0)
            {
                m_CurrentDirection = m_InputQueue.Dequeue();
            }

            Vector2 head = m_SnakeBody[m_SnakeBody.Count - 1];
            Vector2 newHead = head;

            switch (m_CurrentDirection)
            {
                case Direction.Up:    newHead.Y += 1; break;
                case Direction.Down:  newHead.Y -= 1; break;
                case Direction.Left:  newHead.X -= 1; break;
                case Direction.Right: newHead.X += 1; break;
            }

            // 碰撞检测逻辑... (保持你原来的代码逻辑)
            if (newHead.X < 0 || newHead.X >= GridWidth || newHead.Y < 0 || newHead.Y >= GridHeight)
            {
                OnGameOver(); return;
            }

            for (int i = 1; i < m_SnakeBody.Count; i++)
            {
                if ((int)m_SnakeBody[i].X == (int)newHead.X && (int)m_SnakeBody[i].Y == (int)newHead.Y)
                {
                    OnGameOver(); return;
                }
            }

            bool ateFood = ((int)newHead.X == (int)m_FoodPosition.X && (int)newHead.Y == (int)m_FoodPosition.Y);
            
            // 更新表现层
            m_SnakeEntities[m_SnakeEntities.Count - 1].GetComponent<SpriteRendererComponent>().Color = BodyColor;
            m_SnakeBody.Add(newHead);
            m_SnakeEntities.Add(CreateSegmentEntity(newHead, HeadColor));

            if (ateFood)
            {
                m_Score++;
                UpdateScoreText();
                m_CurrentMoveInterval = Math.Max(0.05f, m_CurrentMoveInterval - 0.003f);
                SpawnFood();
            }
            else
            {
                Scene.DestroyEntity(m_SnakeEntities[0]);
                m_SnakeEntities.RemoveAt(0);
                m_SnakeBody.RemoveAt(0);
            }
        }

        // ========== 辅助方法 (保持不变) ==========
        // ... 此处省略 SpawnFood, CreateBorder, GridToWorld 等未变动的辅助方法 ...
        // [请保留你原始代码中剩下的部分]
        
        private void ClearOldEntities()
        {
            for (int i = 0; i < m_SnakeEntities.Count; i++) Scene.DestroyEntity(m_SnakeEntities[i]);
            m_SnakeEntities.Clear();
            m_SnakeBody.Clear();
            if (m_FoodEntity != null) { Scene.DestroyEntity(m_FoodEntity); m_FoodEntity = null; }
        }

        private void InitializeSnake()
        {
            int startX = GridWidth / 2; int startY = GridHeight / 2;
            for (int i = 2; i >= 0; i--)
            {
                Vector2 pos = new Vector2(startX - i, startY);
                m_SnakeBody.Add(pos);
                m_SnakeEntities.Add(CreateSegmentEntity(pos, i == 0 ? HeadColor : BodyColor));
            }
        }

        private void OnGameOver()
        {
            m_GameOver = true;
            foreach (var entity in m_SnakeEntities)
                if (entity.HasComponent<SpriteRendererComponent>())
                    entity.GetComponent<SpriteRendererComponent>().Color = GameOverColor;
            m_ScoreText.Text = $"Game Over! Score: {m_Score}  Press R to Restart";
        }

        private Entity CreateSegmentEntity(Vector2 gridPos, Vector4 color)
        {
            Entity segment = Scene.CreateEntity("SnakeSegment");
            var transform = segment.GetComponent<TransformComponent>();
            transform.Translation = GridToWorld(gridPos);
            transform.Scale = new Vector3(CellSize * 0.9f, CellSize * 0.9f, 1.0f);
            segment.AddComponent<SpriteRendererComponent>().Color = color;
            return segment;
        }

        private Vector3 GridToWorld(Vector2 gridPos)
        {
            float offsetX = -GridWidth * CellSize * 0.5f + CellSize * 0.5f;
            float offsetY = -GridHeight * CellSize * 0.5f + CellSize * 0.5f;
            return new Vector3(gridPos.X * CellSize + offsetX, gridPos.Y * CellSize + offsetY, 0.0f);
        }

        private void UpdateScoreText() => m_ScoreText.Text = $"Score: {m_Score}";

        private void CreateBorder()
        {
            float halfW = GridWidth * CellSize * 0.5f;
            float halfH = GridHeight * CellSize * 0.5f;
            float thickness = CellSize * 0.15f;
            CreateBorderSegment("BB", new Vector3(0, -halfH - thickness * 0.5f, 0), new Vector3(GridWidth * CellSize + thickness * 2, thickness, 1));
            CreateBorderSegment("BT", new Vector3(0, halfH + thickness * 0.5f, 0), new Vector3(GridWidth * CellSize + thickness * 2, thickness, 1));
            CreateBorderSegment("BL", new Vector3(-halfW - thickness * 0.5f, 0, 0), new Vector3(thickness, GridHeight * CellSize + thickness * 2, 1));
            CreateBorderSegment("BR", new Vector3(halfW + thickness * 0.5f, 0, 0), new Vector3(thickness, GridHeight * CellSize + thickness * 2, 1));
        }

        private void CreateBorderSegment(string name, Vector3 position, Vector3 scale)
        {
            Entity border = Scene.CreateEntity(name);
            border.GetComponent<TransformComponent>().Translation = position;
            border.GetComponent<TransformComponent>().Scale = scale;
            border.AddComponent<SpriteRendererComponent>().Color = BorderColor;
        }

        private void SpawnFood()
        {
            List<Vector2> emptyPositions = new List<Vector2>();
            for (int x = 0; x < GridWidth; x++)
                for (int y = 0; y < GridHeight; y++)
                    if (!IsPositionOccupied(x, y)) emptyPositions.Add(new Vector2(x, y));

            if (emptyPositions.Count == 0) { OnGameOver(); return; }
            m_FoodPosition = emptyPositions[m_Random.Next(emptyPositions.Count)];
            if (m_FoodEntity != null) Scene.DestroyEntity(m_FoodEntity);
            m_FoodEntity = Scene.CreateEntity("Food");
            var transform = m_FoodEntity.GetComponent<TransformComponent>();
            transform.Translation = GridToWorld(m_FoodPosition);
            transform.Scale = new Vector3(CellSize * 0.8f, CellSize * 0.8f, 1.0f);
            m_FoodEntity.AddComponent<SpriteRendererComponent>().Color = FoodColor;
        }

        private bool IsPositionOccupied(int x, int y)
        {
            foreach (var pos in m_SnakeBody) if ((int)pos.X == x && (int)pos.Y == y) return true;
            return false;
        }
    }
}