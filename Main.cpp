#include <Siv3D.hpp>
#include <random>

class Anime
{
    //スプライトシートでアニメを再生するクラス
    public:
    Anime(const Texture& texture, int size, int frame) :
    m_texture(texture),
    m_size(size),
    m_frame(frame),
    m_index(0),
    m_count(0) {}
    
    void update()
    {
        ++m_count;
        
        if (m_count > m_frame)
        {
            m_count = 0;
            ++m_index;
            
            if (m_index >= m_size)
            {
                m_index = 0;
            }
        }
    }
    
    void draw(const Vec2& pos, double scale, bool flip) const
    {
        if (flip)
        {
            m_texture.uv(static_cast<double>(m_index) / m_size, 0.0, 1.0 / m_size, 1.0).mirrored().scaled(scale).draw(pos);
        }
        else
        {
            m_texture.uv(static_cast<double>(m_index) / m_size, 0.0, 1.0 / m_size, 1.0).scaled(scale).draw(pos);
        }
    }
    
    // アニメーションが終了したかどうかを判断するメソッド
    bool finished() const
    {
        return m_index == m_size - 1 && m_count == m_frame;
    }
    
    private:
    Texture m_texture;
    int m_size;
    int m_frame;
    int m_index;
    int m_count;
};

class Enemy
{
    public:
    Enemy(const Anime& anime, const Vec2& pos, double speed)
    : m_anime(anime)
    , m_pos(pos.x, 340)
    , m_speed(speed) {}
    
    void update(const Vec2& targetPos)
    {
        // 敵をプレイヤーの方向に移動させる
        double directionX = (targetPos.x - m_pos.x) > 0 ? 1 : -1;
        m_pos.x += directionX * m_speed;
        // アニメーションを更新
        m_anime.update();
    }
    bool operator==(const Enemy& other) const
    {
        return m_pos == other.m_pos;
    }
    
    void draw(const Vec2& targetPos) const
    {
        // Anime クラスの draw 関数に引数を追加
        m_anime.draw(m_pos, 3.0, targetPos.x < m_pos.x);
    }
    Vec2 getPos() const
    {
        return m_pos;
    }
    
    private:
    Anime m_anime;
    Vec2 m_pos;
    double m_speed;
};

void Main()
{
    Window::Resize(873, 525);
    const Font font{ 24, Resource(U"engine/font/x14y24pxHeadUpDaisy.ttf")};
    const Audio slash{ Resource(U"engine/se_hit_004.wav") };
    const Audio disappear{ Resource(U"engine/disappear02.mp3") };
    
    // 敵のHP
    int enemyHP = 3;
    
    // 目標撃破数
    int goal = 10;
    
    // 敵がダメージを受けているかどうかを示すフラグ
    bool enemyDamaged = false;
    
    // 敵が死亡しているかどうかを示すフラグ
    bool isEnemyDead = false;
    
    // 敵の撃破数
    int enemyDefeatedCount = 0;
    
    // テクスチャ読み込み
    Texture groundTexture{Resource(U"engine/texture/ground.png")};
    
    Texture playerTexture{Resource(U"engine/texture/player_walk.png")};
    Texture playerAttackTexture{Resource(U"engine/texture/player_attack.png")};
    
    Texture enemyTexture{Resource(U"engine/texture/enemy_walk.png")};
    Texture enemyDamageTexture{Resource(U"engine/texture/enemy_damage.png")};
    Texture enemyDeathTexture{Resource(U"engine/texture/enemy_death.png")};
    
    
    // アニメーション設定
    Anime enemyAnime(enemyTexture, 9, 10);
    Anime enemyDeathAnime(enemyDeathTexture, 8,10);
    Anime enemyDamageAnime(enemyDamageTexture, 4, 10);
    
    Anime playerAnime(playerTexture, 8, 10);
    Anime playerAttackAnime(playerAttackTexture, 11, 2);
    
    constexpr int32 groundHeight = 150;
    Vec2 playerPos(Scene::Size().x / 2, Scene::Size().y - groundHeight - playerTexture.height());
    constexpr double moveSpeed = 4.0;
    
    // 敵キャラクターのインスタンスを作成
    Vec2 initialEnemyPos = RandomBool() ? Vec2(0, Scene::Height() - groundHeight - enemyTexture.height()) : Vec2(Scene::Width() - enemyTexture.width(), Scene::Height() - groundHeight - enemyTexture.height()+100);
    double enemySpeed = 0.5;
    Enemy enemy(enemyAnime, initialEnemyPos, enemySpeed);
    
    double playerScale = 2.0;
    bool isPlayerFacingLeft = false;
    bool isAttacking = false;
    
    
    while (System::Update())
    {
        // 背景
        groundTexture.scaled(3.0).draw(0, 0);
        // UI
        font(U"Goal:", goal).draw(10, 10, ColorF{ 0.5, 1.0, 0.5 });
        font(U"Kill:", enemyDefeatedCount).draw(10, 46, ColorF{ 0.5, 1.0, 0.5 });
        font(U"enHP:", enemyHP).draw(10, 80, ColorF{ 0.5, 1.0, 0.5 });
        
        // 当たり判定
        Rect playerRect;
        if (isAttacking) {
            if (isPlayerFacingLeft) {
                // 左向きの攻撃
                playerRect = Rect(playerPos.x+100, playerPos.y+100, 60, 25);
                
            } else {
                // 右向きの攻撃
                playerRect = Rect(playerPos.x+170, playerPos.y+100, 60, 25);
                
            }
            
        } else {
            // 通常時の当たり判定
            playerRect = Rect(playerPos.x+150, playerPos.y+60, 35, 50);
            
        }
        //敵アニメ更新
        enemy.update(playerPos);
        // 敵当たり判定の生成
        Rect enemyRect(enemy.getPos().x + 80, enemy.getPos().y + 40, 30, 50);
        // プレイヤーと敵が衝突しているかどうかを判断
        bool isColliding = playerRect.intersects(enemyRect);
        
        // 敵HPが0になったら死亡扱いにする
        if(enemyHP==0)
        {
            isEnemyDead = true;
        }
        
        // 敵が死亡している場合
        if (isEnemyDead)
        {
            enemyDeathAnime.update();
            enemyDeathAnime.draw(enemy.getPos(), 3.0, false);
            disappear.play();
            
            // 死亡アニメーションが終了したら
            if (enemyDeathAnime.finished())
            {
                // 敵のHPをリセット
                enemyHP = 3;
                
                // 敵の死亡状態をリセット
                isEnemyDead = false;
                
                // 撃破数を増やす
                enemyDefeatedCount++;
            }
        }
        else
        {
            //敵がダメージを受けたら
            if (enemyDamaged)
            {
                enemyDamageAnime.update();
                //敵のダメージモーションを描画
                enemyDamageAnime.draw(enemy.getPos(), 3.0, false);
                
                //敵のダメージアニメが終わったらフラグをリセット
                if (enemyDamageAnime.finished())
                {
                    enemyDamaged = false;
                }
            }
            else
            {
                enemy.draw(playerPos); // 敵の通常モーションを描画
            }
        }
        // プレイヤーと敵が衝突している場合
        if (isAttacking && isColliding && ((isPlayerFacingLeft && enemy.getPos().x < playerPos.x) || (!isPlayerFacingLeft && enemy.getPos().x > playerPos.x)))
        {
            if (!enemyDamaged)
            {
                enemyDamaged = true;
                enemyHP--; // 敵のHPを1減らす
            }
            enemyDamageAnime.update();
            enemyDamageAnime.draw(enemy.getPos(), 3.0, isPlayerFacingLeft);
        }
        
        //playerRect.draw(ColorF(0.0, 1.0, 0.0, 0.5)); // 緑色でプレイヤーの矩形を描画
        //enemyRect.draw(ColorF(1.0, 0.0, 0.0, 0.5)); // 赤色で敵の矩形を描画
        
        if (KeySpace.down())
        {
            isAttacking = true;
            playerAttackAnime.update(); // 攻撃アニメーションを初期状態にリセット
        }
        
        //プレイヤーが攻撃中
        if (isAttacking)
        {
            playerAttackAnime.update();
            slash.play();
            playerAttackAnime.draw(playerPos, playerScale, isPlayerFacingLeft);
            
            if (playerAttackAnime.finished()) // 攻撃アニメが終わったとき
            {
                isAttacking = false;
            }
        }
        else
        {
            //キー操作による移動
            if (KeyLeft.pressed())
            {
                playerPos.x -= moveSpeed;
                playerAnime.update();
                isPlayerFacingLeft = true; // 左向きに設定
                playerAnime.draw(playerPos, playerScale, isPlayerFacingLeft);
            }
            else if (KeyRight.pressed())
            {
                playerPos.x += moveSpeed;
                playerAnime.update();
                isPlayerFacingLeft = false; // 右向きに設定
                playerAnime.draw(playerPos, playerScale, isPlayerFacingLeft);
            }
            else
            {
                playerAnime.update();
                playerAnime.draw(playerPos, playerScale, isPlayerFacingLeft);
            }
        }
        if (enemyDefeatedCount >= goal)
        {
            // ゲームクリア処理
            font(U"Game Clear!\n Press Enter Key").draw(400, 250, ColorF{ 1.0, 0.5, 0.5 });
            if (KeyEnter.down())
            {
                // 初期状態に戻る
                enemyDefeatedCount = 0;
                enemyHP = 3;
                isEnemyDead = false;
                enemyDamaged = false;
                isAttacking = false;
                playerPos = Vec2(Scene::Size().x / 2, Scene::Size().y - groundHeight - playerTexture.height());
                initialEnemyPos = RandomBool() ? Vec2(0, Scene::Height() - groundHeight - enemyTexture.height()) : Vec2(Scene::Width() - enemyTexture.width(), Scene::Height() - groundHeight - enemyTexture.height());
                enemy = Enemy(enemyAnime, initialEnemyPos, enemySpeed);
            }
        }
    }
}








