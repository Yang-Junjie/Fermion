using System;
using System.Collections.Generic;
using Fermion;

namespace Photon
{
    public class SnakeGame : Entity
    {
        // --- 可配置参数 ---
        public int GridWidth = 20;
        public int GridHeight = 20;
        public float CellSize = 1.0f;
        public float MoveInterval = 0.15f; // 蛇移动间隔（秒）

        // --- 颜色 ---
        private readonly Vector4 HeadColor = new Vector4(0.2f, 0.8f, 0.2f, 1.0f);   // 蛇头：亮绿
        private readonly Vector4 BodyColor = new Vector4(0.1f, 0.6f, 0.1f, 1.0f);   // 蛇身：深绿
        private readonly Vector4 FoodColor = new Vector4(0.9f, 0.2f, 0.2f, 1.0f);   // 食物：红色
        private readonly Vector4 BorderColor = new Vector4(0.5f, 0.5f, 0.5f, 1.0f); // 边界：灰色
        private readonly Vector4 GameOverColor = new Vector4(0.8f, 0.1f, 0.1f, 1.0f);

        // --- 方向 ---
        private enum Direction { Up, Down, Left, Right }

        // --- 游戏状态 ---
        private List<Vector2> m_SnakeBody = new List<Vector2>();       // 蛇身体坐标（网格坐标）
        private List<Entity> m_SnakeEntities = new List<Entity>();     // 蛇身体对应的实体
        private Vector2 m_FoodPosition;
        private Entity m_FoodEntity;
        private Direction m_CurrentDirection = Direction.Right;
        private Direction m_NextDirection = Direction.Right;           // 缓冲方向，防止快速按键导致反向
        private float m_MoveTimer = 0.0f;
        private float m_CurrentMoveInterval;
        private bool m_GameOver = false;
        private int m_Score = 0;
        private Random m_Random = new Random();
        private Entity m_ScoreEntity;
        private TextComponent m_ScoreText;

        // 按键边缘检测
        private bool m_LastUpDown = false;
        private bool m_LastDownDown = false;
        private bool m_LastLeftDown = false;
        private bool m_LastRightDown = false;
        private bool m_LastRDown = false;

        public void OnCreate()
        {
            // 创建分数文字实体，放在地图边界外左上角
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

            if (m_GameOver)
                return;

            m_MoveTimer += ts;
            if (m_MoveTimer >= m_CurrentMoveInterval)
            {
                m_MoveTimer = 0.0f;
                MoveSnake();
            }
        }

        // ========== 游戏初始化 ==========

        private void StartGame()
        {
            // 销毁旧蛇体
            for (int i = 0; i < m_SnakeEntities.Count; i++)
                Scene.DestroyEntity(m_SnakeEntities[i]);
            m_SnakeEntities.Clear();
            m_SnakeBody.Clear();

            // 销毁旧食物
            if (m_FoodEntity != null)
            {
                Scene.DestroyEntity(m_FoodEntity);
                m_FoodEntity = null;
            }

            // 初始化蛇（3节，从中间开始向右）
            int startX = GridWidth / 2;
            int startY = GridHeight / 2;
            for (int i = 2; i >= 0; i--)
            {
                Vector2 pos = new Vector2(startX - i, startY);
                m_SnakeBody.Add(pos);
                bool isHead = (i == 0);
                Entity segment = CreateSegmentEntity(pos, isHead ? HeadColor : BodyColor);
                m_SnakeEntities.Add(segment);
            }

            m_CurrentDirection = Direction.Right;
            m_NextDirection = Direction.Right;
            m_MoveTimer = 0.0f;
            m_CurrentMoveInterval = MoveInterval;
            m_GameOver = false;
            m_Score = 0;
            UpdateScoreText();

            SpawnFood();
        }

        // ========== 输入处理 ==========

        private void HandleInput()
        {
            // 使用边缘检测，每次按键只触发一次方向改变
            bool upDown = Input.IsKeyDown(KeyCode.Up) || Input.IsKeyDown(KeyCode.W);
            bool downDown = Input.IsKeyDown(KeyCode.Down) || Input.IsKeyDown(KeyCode.S);
            bool leftDown = Input.IsKeyDown(KeyCode.Left) || Input.IsKeyDown(KeyCode.A);
            bool rightDown = Input.IsKeyDown(KeyCode.Right) || Input.IsKeyDown(KeyCode.D);
            bool rDown = Input.IsKeyDown(KeyCode.R);

            // 方向输入（禁止直接反向），方向改变时立即移动
            if (upDown && !m_LastUpDown && m_CurrentDirection != Direction.Down)
            {
                m_NextDirection = Direction.Up;
                m_MoveTimer = m_CurrentMoveInterval;
            }
            else if (downDown && !m_LastDownDown && m_CurrentDirection != Direction.Up)
            {
                m_NextDirection = Direction.Down;
                m_MoveTimer = m_CurrentMoveInterval;
            }
            else if (leftDown && !m_LastLeftDown && m_CurrentDirection != Direction.Right)
            {
                m_NextDirection = Direction.Left;
                m_MoveTimer = m_CurrentMoveInterval;
            }
            else if (rightDown && !m_LastRightDown && m_CurrentDirection != Direction.Left)
            {
                m_NextDirection = Direction.Right;
                m_MoveTimer = m_CurrentMoveInterval;
            }

            // R键重新开始
            if (rDown && !m_LastRDown)
                StartGame();

            m_LastUpDown = upDown;
            m_LastDownDown = downDown;
            m_LastLeftDown = leftDown;
            m_LastRightDown = rightDown;
            m_LastRDown = rDown;
        }

        // ========== 蛇移动逻辑 ==========

        private void MoveSnake()
        {
            m_CurrentDirection = m_NextDirection;

            // 计算新蛇头位置
            Vector2 head = m_SnakeBody[m_SnakeBody.Count - 1];
            Vector2 newHead = head;

            switch (m_CurrentDirection)
            {
                case Direction.Up:    newHead.Y += 1; break;
                case Direction.Down:  newHead.Y -= 1; break;
                case Direction.Left:  newHead.X -= 1; break;
                case Direction.Right: newHead.X += 1; break;
            }

            // 碰撞检测：边界
            if (newHead.X < 0 || newHead.X >= GridWidth ||
                newHead.Y < 0 || newHead.Y >= GridHeight)
            {
                OnGameOver();
                return;
            }

            // 碰撞检测：自身（排除尾巴，因为尾巴即将移走）
            for (int i = 1; i < m_SnakeBody.Count; i++)
            {
                if ((int)m_SnakeBody[i].X == (int)newHead.X &&
                    (int)m_SnakeBody[i].Y == (int)newHead.Y)
                {
                    OnGameOver();
                    return;
                }
            }

            // 检测是否吃到食物
            bool ateFood = ((int)newHead.X == (int)m_FoodPosition.X &&
                            (int)newHead.Y == (int)m_FoodPosition.Y);

            // 旧蛇头变为身体颜色
            Entity oldHeadEntity = m_SnakeEntities[m_SnakeEntities.Count - 1];
            oldHeadEntity.GetComponent<SpriteRendererComponent>().Color = BodyColor;

            // 添加新蛇头
            m_SnakeBody.Add(newHead);
            Entity newHeadEntity = CreateSegmentEntity(newHead, HeadColor);
            m_SnakeEntities.Add(newHeadEntity);

            if (ateFood)
            {
                // 吃到食物：不移除尾巴（蛇变长），重新生成食物
                m_Score++;
                UpdateScoreText();

                // 逐渐加速（最低间隔0.05秒）
                m_CurrentMoveInterval = Math.Max(0.05f, m_CurrentMoveInterval - 0.003f);

                SpawnFood();
            }
            else
            {
                // 没吃到食物：销毁尾巴
                Scene.DestroyEntity(m_SnakeEntities[0]);
                m_SnakeEntities.RemoveAt(0);
                m_SnakeBody.RemoveAt(0);
            }
        }

        // ========== 食物生成 ==========

        private void SpawnFood()
        {
            // 在空位生成食物
            List<Vector2> emptyPositions = new List<Vector2>();
            for (int x = 0; x < GridWidth; x++)
            {
                for (int y = 0; y < GridHeight; y++)
                {
                    bool occupied = false;
                    for (int i = 0; i < m_SnakeBody.Count; i++)
                    {
                        if ((int)m_SnakeBody[i].X == x && (int)m_SnakeBody[i].Y == y)
                        {
                            occupied = true;
                            break;
                        }
                    }
                    if (!occupied)
                        emptyPositions.Add(new Vector2(x, y));
                }
            }

            if (emptyPositions.Count == 0)
            {
                OnGameOver();
                return;
            }

            m_FoodPosition = emptyPositions[m_Random.Next(emptyPositions.Count)];

            if (m_FoodEntity != null)
                Scene.DestroyEntity(m_FoodEntity);

            m_FoodEntity = Scene.CreateEntity("Food");
            TransformComponent foodTransform = m_FoodEntity.GetComponent<TransformComponent>();
            foodTransform.Translation = GridToWorld(m_FoodPosition);
            foodTransform.Scale = new Vector3(CellSize * 0.8f, CellSize * 0.8f, 1.0f);

            SpriteRendererComponent foodSprite = m_FoodEntity.AddComponent<SpriteRendererComponent>();
            foodSprite.Color = FoodColor;
        }

        // ========== 游戏结束 ==========

        private void OnGameOver()
        {
            m_GameOver = true;

            // 蛇全部变红
            for (int i = 0; i < m_SnakeEntities.Count; i++)
            {
                if (m_SnakeEntities[i].HasComponent<SpriteRendererComponent>())
                    m_SnakeEntities[i].GetComponent<SpriteRendererComponent>().Color = GameOverColor;
            }

            m_ScoreText.Text = $"Game Over! Score: {m_Score}  Press R to Restart";
            Utils.Log($"Game Over! Final Score: {m_Score}");
        }

        // ========== 辅助方法 ==========

        private Entity CreateSegmentEntity(Vector2 gridPos, Vector4 color)
        {
            Entity segment = Scene.CreateEntity("SnakeSegment");
            TransformComponent transform = segment.GetComponent<TransformComponent>();
            transform.Translation = GridToWorld(gridPos);
            transform.Scale = new Vector3(CellSize * 0.9f, CellSize * 0.9f, 1.0f);

            SpriteRendererComponent sprite = segment.AddComponent<SpriteRendererComponent>();
            sprite.Color = color;

            return segment;
        }

        private Vector3 GridToWorld(Vector2 gridPos)
        {
            // 将网格坐标转为世界坐标，以网格中心为原点
            float offsetX = -GridWidth * CellSize * 0.5f + CellSize * 0.5f;
            float offsetY = -GridHeight * CellSize * 0.5f + CellSize * 0.5f;
            return new Vector3(gridPos.X * CellSize + offsetX, gridPos.Y * CellSize + offsetY, 0.0f);
        }

        private void CreateBorder()
        {
            // 创建四条边界
            float halfW = GridWidth * CellSize * 0.5f;
            float halfH = GridHeight * CellSize * 0.5f;
            float thickness = CellSize * 0.15f;

            // 下
            CreateBorderSegment("BorderBottom",
                new Vector3(0, -halfH - thickness * 0.5f, 0),
                new Vector3(GridWidth * CellSize + thickness * 2, thickness, 1));
            // 上
            CreateBorderSegment("BorderTop",
                new Vector3(0, halfH + thickness * 0.5f, 0),
                new Vector3(GridWidth * CellSize + thickness * 2, thickness, 1));
            // 左
            CreateBorderSegment("BorderLeft",
                new Vector3(-halfW - thickness * 0.5f, 0, 0),
                new Vector3(thickness, GridHeight * CellSize + thickness * 2, 1));
            // 右
            CreateBorderSegment("BorderRight",
                new Vector3(halfW + thickness * 0.5f, 0, 0),
                new Vector3(thickness, GridHeight * CellSize + thickness * 2, 1));
        }

        private void CreateBorderSegment(string name, Vector3 position, Vector3 scale)
        {
            Entity border = Scene.CreateEntity(name);
            TransformComponent transform = border.GetComponent<TransformComponent>();
            transform.Translation = position;
            transform.Scale = scale;

            SpriteRendererComponent sprite = border.AddComponent<SpriteRendererComponent>();
            sprite.Color = BorderColor;
        }

        private void UpdateScoreText()
        {
            m_ScoreText.Text = $"Score: {m_Score}";
        }
    }
}
