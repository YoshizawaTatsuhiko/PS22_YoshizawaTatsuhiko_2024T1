# include <Siv3D.hpp>

/*
	古き良き書き方での実装
	・安全性や利便性などは一切考えていない
*/

/// @brief ブロックのサイズ
constexpr Size BRICK_SIZE{ 40, 20 };

/// @brief ボールの速さ
constexpr double BALL_SPEED = 500.0;

/// @brief ブロックの数　縦
constexpr int Y_COUNT = 7;

/// @brief ブロックの数　横
constexpr int X_COUNT = 20;

/// @brief 合計ブロック数
constexpr int MAX = Y_COUNT * X_COUNT;

/// @brief 不正な座標
constexpr Point WRONG_POINT = Point(-1, -1);

void Main()
{
#pragma region Game Control
	bool is_game_start = false;
	bool is_game_finish = false;
#pragma endregion

#pragma region Paddle
	int paddle_x = Scene::Width() / 2;
#pragma endregion

#pragma region Ball
	/// @brief ボール
	int ball_radius = 8;
	Circle ball{ 400, 400, ball_radius };
	/// @brief ボールの速度
	Vec2 ball_velocity{ Cos(Random(45, 135)) * BALL_SPEED, BALL_SPEED};
#pragma endregion

#pragma region Bricks
	/// @brief ブロック
	Rect bricks[MAX];
	/// @brief 衝突したブロックの座標
	Point intersect_block_pos;

	// ブロックを初期化
	for (int y = 0; y < Y_COUNT; ++y) {
		for (int x = 0; x < X_COUNT; ++x) {
			int index = y * X_COUNT + x;
			bricks[index] = Rect{
				x * BRICK_SIZE.x,
				60 + y * BRICK_SIZE.y,
				BRICK_SIZE
			};
		}
	}
#pragma endregion

#pragma region UI
	String text_message = U"";
	Font text{FontMethod::MSDF, 40};
	Effect effect;
#pragma endregion


	while (System::Update())
	{
		//==============================
		// 更新
		//==============================

		// パドル
		const Rect paddle{ Arg::center(paddle_x, 500), 60, 10 };

		// パドルが画面外に行かないようにする
		if (Cursor::Pos().x - paddle.size.x / 2 < 0) {
			paddle_x = paddle.size.x / 2;
		}
		else if (Cursor::Pos().x + paddle.size.x / 2 > Scene::Width()) {
			paddle_x = Scene::Width() - paddle.size.x / 2;
		}
		else {
			paddle_x = Cursor::Pos().x;
		}

		// ゲーム開始時の入力受付待ち
		if (!is_game_start && MouseL.down()) {
			is_game_start = true;
		}

		// 入力があるまで、ボールを動かさない
		if (is_game_start) {
			// ボール移動
			ball.moveBy(ball_velocity * Scene::DeltaTime());
		}
		else {
			ball.setPos(paddle.centerX(), paddle.centerY() - (ball_radius + paddle.size.y));
		}

		// リスタート処理
		if (is_game_finish && MouseR.down()) {
			// ブロックを初期化
			for (int y = 0; y < Y_COUNT; ++y) {
				for (int x = 0; x < X_COUNT; ++x) {
					int index = y * X_COUNT + x;
					bricks[index].y = 60 + y * BRICK_SIZE.y;
				}
			}
			// 変数を初期化
			is_game_start = false;
			is_game_finish = false;
		}

		//==============================
		// コリジョン
		//==============================

		// ブロックとの衝突を検知
		intersect_block_pos = WRONG_POINT;

		for (int i = 0; i < MAX; ++i) {
			// 参照で保持
			Rect& refBrick = bricks[i];

			// 衝突を検知
			if (refBrick.intersects(ball))
			{
				// ブロックの上辺、または底辺と交差
				if (refBrick.bottom().intersects(ball) || refBrick.top().intersects(ball))
				{
					ball_velocity.y *= -1;
				}
				else // ブロックの左辺または右辺と交差
				{
					ball_velocity.x *= -1;
				}

				intersect_block_pos = refBrick.center().asPoint();
				// あたったブロックは画面外に出す
				refBrick.y -= 600;

				// 同一フレームでは複数のブロック衝突を検知しない
				break;
			}
		}

		// 天井との衝突を検知
		if ((ball.y < 0) && (ball_velocity.y < 0))
		{
			ball_velocity.y *= -1;
		}

		// 壁との衝突を検知
		if (((ball.x < 0) && (ball_velocity.x < 0))
			|| ((Scene::Width() < ball.x) && (0 < ball_velocity.x)))
		{
			ball_velocity.x *= -1;
		}

		// パドルとの衝突を検知
		if ((0 < ball_velocity.y) && paddle.intersects(ball))
		{
			ball_velocity = Vec2{
				(ball.x - paddle.center().x) * 10,
				-ball_velocity.y
			}.setLength(BALL_SPEED);
		}

		//==============================
		// 描画
		//==============================
		// ブロック描画
		for (int i = 0; i < MAX; ++i) {
			bricks[i].stretched(-1).draw(HSV{ bricks[i].y - 40 });
		}

		// ボール描画
		ball.draw();

		// パドル描画
		paddle.rounded(3).draw();

		// テキスト描画
		if (!is_game_start) {
			text_message = U"Press Left_Mouse_Button";
		}
		else if (ball.y > Scene::Height()) {
			text_message = U"Game Over\nPress Right_Mouse_Button";
			is_game_finish = true;
		}
		else {
			text_message = U"";
		}
		text(text_message).drawAt(40, Scene::Center(), Color(Palette::White));

		// 描画するパーティクルを追加する
		if (intersect_block_pos != WRONG_POINT) {
			effect.add([pos = intersect_block_pos, color = RandomColorF()](double t)
			{
				Rect{ Arg::center(pos), BRICK_SIZE }.drawFrame(4, color);
				return (t < 0.20);
			});
		}
		
		// パーティクル描画
		effect.update();
	}
}
