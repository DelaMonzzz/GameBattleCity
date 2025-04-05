#include <iostream>        // Thư viện để nhập/xuất dữ liệu (cin, cout)
#include <algorithm>       // Thư viện hỗ trợ các thuật toán như sort, find, remove_if
#include <SDL.h>           // Thư viện SDL để xử lý đồ họa 2D và sự kiện
#include <vector>          // Thư viện hỗ trợ cấu trúc dữ liệu vector

using namespace std;       // Dùng không gian tên std để sử dụng các đối tượng từ thư viện chuẩn

const int SCREEN_WIDTH = 800;  // Chiều rộng của màn hình
const int SCREEN_HEIGHT = 600; // Chiều cao của màn hình
const int TILE_SIZE = 40;     // Kích thước của một ô trong bản đồ
const int MAP_WIDTH = SCREEN_WIDTH / TILE_SIZE;  // Số lượng ô ngang trong bản đồ
const int MAP_HEIGHT = SCREEN_HEIGHT / TILE_SIZE; // Số lượng ô dọc trong bản đồ

// Định nghĩa lớp Wall đại diện cho các bức tường trong trò chơi
class Wall {
public:
    int x, y;              // Vị trí của bức tường
    SDL_Rect rect;         // Hình chữ nhật đại diện cho bức tường
    bool active;           // Trạng thái hoạt động của bức tường (có bị phá hủy hay không)

    Wall(int startX, int startY) {   // Constructor khởi tạo bức tường tại vị trí (startX, startY)
        x = startX;
        y = startY;
        active = true;                // Bức tường được kích hoạt
        rect = {x, y, TILE_SIZE, TILE_SIZE}; // Tạo một hình chữ nhật với kích thước TILE_SIZE
    }

    // Hàm vẽ bức tường lên màn hình
    void render(SDL_Renderer* renderer) {
        if (active) {
            SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255); // Chọn màu nâu cho bức tường
            SDL_RenderFillRect(renderer, &rect);               // Vẽ hình chữ nhật lên renderer
        }
    }
};

// Định nghĩa lớp Bullet đại diện cho viên đạn
class Bullet {
public:
    int x, y;          // Vị trí của viên đạn
    int dx, dy;        // Vận tốc di chuyển của viên đạn theo các trục x và y
    SDL_Rect rect;     // Hình chữ nhật đại diện cho viên đạn
    bool active;       // Trạng thái hoạt động của viên đạn

    Bullet(int startX, int startY, int dirX, int dirY) { // Constructor khởi tạo viên đạn
        x = startX;
        y = startY;
        dx = dirX;
        dy = dirY;
        active = true;
        rect = {x, y, 10, 10}; // Kích thước viên đạn là 10x10
    }

    // Hàm di chuyển viên đạn
    void move() {
        x += dx;          // Cập nhật vị trí theo hướng di chuyển
        y += dy;
        rect.x = x;       // Cập nhật vị trí cho hình chữ nhật
        rect.y = y;
        if (x < TILE_SIZE || x > SCREEN_WIDTH - TILE_SIZE || y < TILE_SIZE || y > SCREEN_HEIGHT - TILE_SIZE) {
            active = false; // Nếu viên đạn ra khỏi màn hình, không còn hoạt động
        }
    }

    // Hàm vẽ viên đạn lên màn hình
    void render(SDL_Renderer* renderer) {
        if (active) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Màu trắng cho viên đạn
            SDL_RenderFillRect(renderer, &rect);                   // Vẽ viên đạn lên màn hình
        }
    }
};

// Định nghĩa lớp PlayerTank đại diện cho chiếc xe tăng của người chơi
class PlayerTank {
public:
    int x, y;          // Vị trí của xe tăng
    int dirX, dirY;    // Hướng di chuyển của xe tăng
    SDL_Rect rect;     // Hình chữ nhật đại diện cho xe tăng
    vector<Bullet> bullets; // Danh sách viên đạn của xe tăng

    PlayerTank() {
        x = 0;           // Khởi tạo xe tăng ở vị trí (0, 0)
        y = 0;
        rect = {x, y, TILE_SIZE, TILE_SIZE};  // Kích thước xe tăng
        dirX = 0;
        dirY = -1;       // Hướng di chuyển mặc định là lên trên
    }

    PlayerTank(int startX, int startY) {  // Constructor cho phép tạo xe tăng ở vị trí tùy ý
        x = startX;
        y = startY;
        rect = {x, y, TILE_SIZE, TILE_SIZE}; // Kích thước xe tăng
        dirX = 0;
        dirY = -1;       // Hướng di chuyển mặc định là lên trên
    }

    // Hàm di chuyển xe tăng
    void move(int dx, int dy, const vector<Wall>& walls) {
        int newX = x + dx;
        int newY = y + dy;
        this->dirX = dx;
        this->dirY = dy;

        SDL_Rect newRect = { newX, newY, TILE_SIZE, TILE_SIZE};
        for (int i = 0; i < walls.size(); i++) {
            if (walls[i].active && SDL_HasIntersection(&newRect, &walls[i].rect)) {
                return; // Nếu xe tăng va vào bức tường, không di chuyển
            }
        }

        // Nếu không va chạm với tường, cập nhật vị trí của xe tăng
        if (newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE * 2 && newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE * 2) {
            x = newX;
            y = newY;
            rect.x = x;
            rect.y = y;
        }
    }

    // Hàm bắn đạn
    void shoot() {
        if (bullets.size() >= 3) { // Nếu số lượng đạn hiện tại đã đạt giới hạn 3 viên, không bắn thêm
        return;
    }
    // Nếu chưa đạt giới hạn, tạo một viên đạn mới
    bullets.push_back(Bullet(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, this->dirX, this->dirY));
    }

    // Cập nhật trạng thái các viên đạn của người chơi
    void updateBullets() {
        for (auto &bullet : bullets) {
            bullet.move(); // Di chuyển viên đạn
        }
        bullets.erase(remove_if(bullets.begin(), bullets.end(), [](Bullet &b) { return !b.active; }), bullets.end()); // Xóa viên đạn không còn hoạt động
    }

    // Vẽ xe tăng và các viên đạn của nó
    void render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Màu vàng cho xe tăng
        SDL_RenderFillRect(renderer, &rect);
        for (auto &bullet : bullets) {
            bullet.render(renderer); // Vẽ các viên đạn
        }
    }
};

// Định nghĩa lớp EnemyTank đại diện cho chiếc xe tăng đối thủ
class EnemyTank {
public:
    int x, y;          // Vị trí của xe tăng
    int dirX, dirY;    // Hướng di chuyển của xe tăng
    int moveDelay, shootDelay;  // Thời gian trì hoãn giữa các lần di chuyển và bắn
    SDL_Rect rect;     // Hình chữ nhật đại diện cho xe tăng
    bool active;       // Trạng thái hoạt động của xe tăng
    vector<Bullet> bullets;  // Danh sách viên đạn của xe tăng đối thủ

    EnemyTank() {
        moveDelay = 5;  // Thời gian trì hoãn giữa các lần di chuyển
        shootDelay = 2;  // Thời gian trì hoãn giữa các lần bắn
        x = 0;
        y = 0;
        rect = {x, y, TILE_SIZE, TILE_SIZE}; // Kích thước xe tăng
        dirX = 0;
        dirY = 1;       // Hướng di chuyển mặc định là xuống dưới
        active = true;
    }

    EnemyTank(int startX, int startY) {  // Constructor cho phép tạo xe tăng đối thủ tại vị trí tùy ý
        moveDelay = 5;
        shootDelay = 2;
        x = startX;
        y = startY;
        rect = {x, y, TILE_SIZE, TILE_SIZE}; // Kích thước xe tăng
        dirX = 0;
        dirY = 1;       // Hướng di chuyển mặc định là xuống dưới
        active = true;
    }

    // Hàm di chuyển xe tăng đối thủ
    void move(const vector<Wall>& walls) {
        if (--moveDelay > 0) return; // Nếu thời gian trì hoãn chưa hết, không di chuyển
        moveDelay = 5; // Cập nhật thời gian trì hoãn
        int r = rand() % 4;  // Chọn một hướng ngẫu nhiên (lên, xuống, trái, phải)
        if (r == 0) { // Di chuyển lên
            this->dirX = 0;
            this->dirY = -5;
        } else if (r == 1) { // Di chuyển xuống
            this->dirX = 0;
            this->dirY = 5;
        } else if (r == 2) { // Di chuyển sang trái
            this->dirX = -5;
            this->dirY = 0;
        } else if (r == 3) { // Di chuyển sang phải
            this->dirX = 5;
            this->dirY = 0;
        }

        int newX = x + this->dirX; // Tính toán vị trí mới của xe tăng
        int newY = y + this->dirY;

        SDL_Rect newRect = { newX, newY, TILE_SIZE, TILE_SIZE };
        for (const auto& wall : walls) { // Kiểm tra va chạm với tường
            if (wall.active && SDL_HasIntersection(&newRect, &wall.rect)) {
                return; // Nếu va vào tường, không di chuyển
            }
        }

        // Nếu không va chạm với tường, cập nhật vị trí của xe tăng
        if (newX >= TILE_SIZE && newX <= SCREEN_WIDTH - TILE_SIZE * 2 && newY >= TILE_SIZE && newY <= SCREEN_HEIGHT - TILE_SIZE * 2) {
            x = newX;
            y = newY;
            rect.x = x;
            rect.y = y;
        }
    }

    // Hàm bắn đạn của xe tăng đối thủ
    void shoot() {
        if (--shootDelay > 0) return; // Nếu thời gian trì hoãn chưa hết, không bắn
        shootDelay = 2;  // Cập nhật thời gian trì hoãn giữa các lần bắn
        bullets.push_back(Bullet(x + TILE_SIZE / 2 - 5, y + TILE_SIZE / 2 - 5, this->dirX, this->dirY)); // Bắn đạn vào vị trí chính giữa xe tăng
    }

    // Cập nhật trạng thái các viên đạn của xe tăng đối thủ
    void updateBullets() {
        for (auto &bullet : bullets) {
            bullet.move(); // Di chuyển viên đạn
        }
        // Xóa các viên đạn không còn hoạt động
        bullets.erase(remove_if(bullets.begin(), bullets.end(), [](Bullet &b) { return !b.active; }), bullets.end());
    }

    // Vẽ xe tăng và các viên đạn của nó lên màn hình
    void render(SDL_Renderer* renderer) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Màu đỏ cho xe tăng
        SDL_RenderFillRect(renderer, &rect);
        for (auto &bullet : bullets) {
            bullet.render(renderer); // Vẽ các viên đạn của xe tăng
        }
    }
};

// Định nghĩa lớp Game, điều khiển toàn bộ trò chơi
class Game {
public:
    SDL_Window* window;     // Cửa sổ hiển thị trò chơi
    SDL_Renderer* renderer; // Bộ vẽ đồ họa
    bool running;           // Trạng thái của trò chơi (chạy hay dừng)
    vector<Wall> walls;     // Danh sách các bức tường
    PlayerTank player;      // Người chơi
    int enemyNumber = 3;    // Số lượng xe tăng đối thủ
    vector<EnemyTank> enemies; // Danh sách các xe tăng đối thủ

    // Hàm tạo ra các bức tường trong bản đồ
    void generateWalls() {
        for (int i = 3; i < MAP_HEIGHT - 3; i += 2) {  // Duyệt qua các ô trong bản đồ
            for (int j = 3; j < MAP_WIDTH - 3; j += 2) { // Duyệt qua các ô trong bản đồ
                Wall w = Wall(j * TILE_SIZE, i * TILE_SIZE); // Tạo bức tường tại vị trí (j, i)
                walls.push_back(w); // Thêm bức tường vào danh sách
            }
        }
    }

    // Hàm sinh các xe tăng đối thủ
    void spawnEnemies() {
        enemies.clear(); // Xóa tất cả các xe tăng đối thủ cũ
        for (int i = 0; i < enemyNumber; ++i) {  // Sinh số lượng xe tăng đối thủ
            int ex, ey;
            bool validPosition = false;
            while (!validPosition) {  // Tạo xe tăng tại một vị trí hợp lệ (không trùng với tường)
                ex = (rand() % (MAP_WIDTH - 2) + 1) * TILE_SIZE;
                ey = (rand() % (MAP_HEIGHT - 2) + 1) * TILE_SIZE;
                validPosition = true;
                for (const auto& wall : walls) {
                    if (wall.active && wall.x == ex && wall.y == ey) {
                        validPosition = false; // Nếu xe tăng trùng với vị trí tường, tạo lại vị trí mới
                        break;
                    }
                }
            }
            enemies.push_back(EnemyTank(ex, ey)); // Thêm xe tăng đối thủ vào danh sách
        }
    }

    // Hàm xử lý các sự kiện (chạy trong vòng lặp game)
    void handleEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {  // Duyệt qua tất cả các sự kiện
            if (event.type == SDL_QUIT) {
                running = false; // Nếu sự kiện là đóng cửa sổ, kết thúc trò chơi
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {  // Kiểm tra phím bấm
                    case SDLK_w: player.move(0, -5, walls); break; // Di chuyển lên
                    case SDLK_s: player.move(0, 5, walls); break; // Di chuyển xuống
                    case SDLK_a: player.move(-5, 0, walls); break; // Di chuyển trái
                    case SDLK_d: player.move(5, 0, walls); break; // Di chuyển phải
                    case SDLK_SPACE: player.shoot(); break; // Bắn đạn
                }
            }
        }
    }

    // Hàm cập nhật trạng thái của trò chơi (di chuyển, bắn, va chạm, ...)
    void update() {
        player.updateBullets(); // Cập nhật trạng thái các viên đạn của người chơi

        for (auto& enemy : enemies) {  // Cập nhật trạng thái các xe tăng đối thủ
            enemy.move(walls);          // Di chuyển xe tăng
            enemy.updateBullets();      // Cập nhật viên đạn của xe tăng đối thủ
            if (rand() % 100 < 2) {     // Xác suất bắn đạn ngẫu nhiên
                enemy.shoot();
            }
        }

        // Kiểm tra va chạm giữa đạn và tường
        for (auto& enemy : enemies) {
            for (auto& bullet : enemy.bullets) {
                for (auto& wall : walls) {
                    if (wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                        wall.active = false; // Nếu đạn va vào tường, phá hủy tường và đạn
                        bullet.active = false;
                        break;
                    }
                }
            }
        }

        // Kiểm tra va chạm giữa đạn và tường của người chơi
        for (auto& bullet : player.bullets) {
            for (auto& wall : walls) {
                if (wall.active && SDL_HasIntersection(&bullet.rect, &wall.rect)) {
                    wall.active = false; // Nếu đạn va vào tường, phá hủy tường và đạn
                    bullet.active = false;
                    break;
                }
            }
        }

        // Kiểm tra va chạm giữa đạn của người chơi và xe tăng đối thủ
        for (auto& bullet : player.bullets) {
            for (auto& enemy : enemies) {
                if (enemy.active && SDL_HasIntersection(&bullet.rect, &enemy.rect)) {
                    enemy.active = false; // Nếu đạn trúng xe tăng đối thủ, phá hủy xe tăng và đạn
                    bullet.active = false;
                }
            }
        }

        // Xóa các xe tăng đối thủ không còn hoạt động
        enemies.erase(remove_if(enemies.begin(), enemies.end(), [](EnemyTank &e) { return !e.active; }), enemies.end());
        if (enemies.empty()) {
            running = false; // Nếu không còn xe tăng đối thủ, kết thúc trò chơi
        }

        // Kiểm tra va chạm giữa đạn của xe tăng đối thủ và người chơi
        for (auto& enemy : enemies) {
            for (auto& bullet : enemy.bullets) {
                if (SDL_HasIntersection(&bullet.rect, &player.rect)) {
                    running = false; // Nếu xe tăng đối thủ bắn trúng người chơi, kết thúc trò chơi
                    return;
                }
            }
        }
    }

    // Constructor khởi tạo trò chơi
    Game() {
        running = true;
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {  // Khởi tạo SDL
            cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
            running = false;
        }
        window = SDL_CreateWindow("Battle City", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN); // Tạo cửa sổ trò chơi
        if (!window) {
            cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
            running = false;
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // Tạo renderer
        if (!renderer) {
            cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
            running = false;
        }
        generateWalls(); // Tạo các bức tường
        player = PlayerTank(((MAP_WIDTH - 1) / 2) * TILE_SIZE, (MAP_HEIGHT - 2) * TILE_SIZE); // Khởi tạo người chơi
        spawnEnemies(); // Tạo các xe tăng đối thủ
    }

    // Hàm vẽ trò chơi lên màn hình
    void render() {
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255); // Màu xám cho phần biên giới
        SDL_RenderClear(renderer); // Xóa màn hình

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Màu đen cho phần nền
        for (int i = 1; i < MAP_HEIGHT - 1; i++) { // Vẽ nền bản đồ
            for (int j = 1; j < MAP_WIDTH - 1; j++) {
                SDL_Rect tile = { j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE };
                SDL_RenderFillRect(renderer, &tile);
            }
        }

        // Vẽ các bức tường
        for (auto& wall : walls) {
            wall.render(renderer);
        }

        // Vẽ người chơi và các viên đạn của người chơi
        player.render(renderer);

        // Vẽ các xe tăng đối thủ và các viên đạn của chúng
        for (auto& enemy : enemies) {
            enemy.render(renderer);
        }

        SDL_RenderPresent(renderer); // Hiển thị màn hình mới
    }

    // Hàm chạy vòng lặp trò chơi
    void run() {
        while (running) {
            handleEvents();  // Xử lý các sự kiện
            update();        // Cập nhật trạng thái trò chơi
            render();        // Vẽ trò chơi lên màn hình
            SDL_Delay(1000 / 60); // Giới hạn FPS (60 FPS)
        }
    }

    // Destructor giải phóng tài nguyên
    ~Game() {
        SDL_DestroyRenderer(renderer); // Hủy renderer
        SDL_DestroyWindow(window);     // Hủy cửa sổ
        SDL_Quit();                    // Đóng SDL
    }
};

int main(int argc, char* argv[]) {
    Game game; // Khởi tạo đối tượng 'game' của lớp 'Game'
    if (game.running) { // Kiểm tra xem trò chơi có đang chạy không
        game.run(); // Nếu trò chơi đang chạy, gọi hàm 'run' để bắt đầu vòng lặp trò chơi
    }
    return 0; // Kết thúc chương trình
}
