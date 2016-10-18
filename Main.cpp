#include <iostream>
#include <vector>
#include <algorithm>
#include <array>
#include <set>
#include <map>
#include <queue>
#include <tuple>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <random>
#include <memory>
#include <cassert>
#define repeat(i,n) for (int i = 0; (i) < (n); ++(i))
#define repeat_from(i,m,n) for (int i = (m); (i) < (n); ++(i))
#define repeat_reverse(i,n) for (int i = (n)-1; (i) >= 0; --(i))
#define repeat_from_reverse(i,m,n) for (int i = (n)-1; (i) >= (m); --(i))
#define whole(f,x,...) ([&](decltype((x)) y) { return (f)(begin(y), end(y), ## __VA_ARGS__); })(x)
typedef long long ll;
using namespace std;
template <class T> void setmax(T & a, T const & b) { if (a < b) a = b; }
template <class T> void setmin(T & a, T const & b) { if (b < a) a = b; }
bool is_on_field(int y, int x, int h, int w) { return 0 <= y and y < h and 0 <= x and x < w; }

struct point_t { int y, x; };
point_t point(int y, int x) { return (point_t) { y, x }; }
template <typename T>
point_t point(T const & p) { return (point_t) { p.y, p.x }; }
bool operator == (point_t a, point_t b) { return make_pair(a.y, a.x) == make_pair(b.y, b.x); }
bool operator != (point_t a, point_t b) { return make_pair(a.y, a.x) != make_pair(b.y, b.x); }
bool operator <  (point_t a, point_t b) { return make_pair(a.y, a.x) <  make_pair(b.y, b.x); }

namespace primitive {
    const int pack_size = 3;
    const int width = 10;
    const int height = 16;
    const int sum = 10;
    const int turn_number = 500;

    typedef char block_t;
    const block_t empty_block = 0;
    const block_t obstacle_block = sum + 1;
    template <int H, int W>
    struct blocks_t {
        array<array<block_t, W>, H> at;
    };
    typedef blocks_t<pack_size, pack_size> pack_t;
    typedef blocks_t<height, width> field_t;
    template <int H, int W>
    istream & operator >> (istream & in, blocks_t<H, W> & a) {
        repeat (y, H) {
            repeat (x, W) {
                int c; in >> c; a.at[y][x] = c;
                assert ((a.at[y][x] == 0 and 0 == empty_block)
                        or (1 <= a.at[y][x] and a.at[y][x] <= 9)
                        or (a.at[y][x] == 11 and 11 == obstacle_block));
            }
        }
        string s; in >> s; assert (s == "END");
        return in;
    }
    template <int H, int W>
    ostream & operator << (ostream & out, blocks_t<H, W> const & a) {
        repeat (y, H) {
            if (y) out << endl;
            repeat (x, W) {
                switch (a.at[y][x]) {
                    case empty_block: out << "  "; break; // 私のJava環境上では空白2つがちょうど良い幅になる
                    case obstacle_block: out << '#'; break;
                    default: out << int(a.at[y][x]); break;
                }
            }
        }
        return out;
    }
    template <int H, int W>
    bool operator == (blocks_t<H, W> const & a, blocks_t<H, W> const & b) {
        return a.at == b.at;
    }

    const int dangerline = height + 1;
    struct config_t {
        // int width, height;
        // int pack_size, sum;
        vector<pack_t> packs;
    };
    istream & operator >> (istream & in, config_t & a) {
        int w, h, t, s, n; in >> w >> h >> t >> s >> n;
        assert (w == width);
        assert (h == height);
        assert (t == pack_size);
        assert (s == sum);
        assert (n == turn_number);
        a.packs.resize(turn_number);
        repeat (i,n) in >> a.packs[i];
        return in;
    }

    struct input_t {
        int current_turn;
        int remaining_time; // in msec
        int self_obstacles;
        field_t self_field;
        int opponent_obstacles;
        field_t opponent_field;
    };
    istream & operator >> (istream & in, input_t & a) {
        in >> a.current_turn >> a.remaining_time;
        in >>     a.self_obstacles >>     a.self_field;
        in >> a.opponent_obstacles >> a.opponent_field;
        assert (a.self_obstacles == 0 or a.opponent_obstacles == 0);
        return in;
    }

    typedef int rotate_t;
    struct output_t {
        int x;
        rotate_t rotate;
    };
    ostream & operator << (ostream & out, output_t const & a) {
        return out << a.x << ' ' << a.rotate;
    }
    output_t make_output(int x, rotate_t rotate) {
        return { x, rotate };
    }
}
using namespace primitive;

pack_t rotate(pack_t a, rotate_t r) {
    repeat (i, (r % 4 + 4) % 4) {
        pack_t b = a;
        repeat (y, pack_size) repeat (x, pack_size) {
            a.at[x][2-y] = b.at[y][x];
        }
    }
    return a;
}

pack_t fill_obstacles(pack_t a, int obstacles) {
    repeat (y, pack_size) {
        repeat (x, pack_size) {
            if (obstacles and a.at[y][x] == empty_block) {
                a.at[y][x] = obstacle_block;
                obstacles -= 1;
            }
        }
    }
    return a;
}

const int chain_coefficient[] = {
    0,
    1, // 1
    1, // 2
    2, // 3
    2, // 4
    3, // 5
    4, // 6
    6, // 7
    8, // 8
    10, // 9
    13, // 10
    17, // 11
    23, // 12
    30, // 13
    39, // 14
    51, // 15
    66, // 16
    86, 112, 146, 190, 247, 321, 417, 542, 705, 917, 1192, 1550, 2015, 2619, 3405, 4427, 5756, 7482, 9727, 12646, 16440, 21372, 27783, 36118, 46954, 61040, 79353, 103159, 134106, 174338, 226640, 294632, 383022, 497929, 647307, 841500, 1093950, 1422135, 1848776, 2403409, 3124432, 4061761, 5280290, 6864377, 8923690, 11600797, 15081036, 19605347, 25486951, 33133037, 43072948, 55994833, 72793283, 94631268, 123020648, 159926843, 207904896, 270276365, 351359275, 456767058, 593797175, 771936328,
    1003517226, // 79
    1304572395, // 80
    1695944113, // 81
};

struct simulate_pack_invalid_output_exception {};
struct simulate_pack_gameover_exception {};
struct simulate_pack_result_t {
    int score;
    field_t field;
};
simulate_pack_result_t simulate_pack(field_t const & field, pack_t const & a_pack, int obstacles, output_t const & output) { // throws exceptions
    blocks_t<height + pack_size, width> workspace;
    auto & at = workspace.at;
    repeat (y, pack_size) repeat (x, width) at[y][x] = empty_block;
    repeat (y,    height) repeat (x, width) at[y + pack_size][x] = field.at[y][x];
    // 1. パックの投下
    pack_t pack = rotate(fill_obstacles(a_pack, obstacles), output.rotate);
    repeat (dy, 3) repeat (dx, 3) if (pack.at[dy][dx] != empty_block) {
        int ny = dy;
        int nx = output.x + dx;
        if (nx < 0 or width <= nx) throw simulate_pack_invalid_output_exception();
        at[ny][nx] = pack.at[dy][dx];
    }
    // 2. ブロックの消滅&落下処理
    int score = 0;
    for (int chain = 1; ; ++ chain) {
        // 落下
        repeat_reverse (y, height + pack_size) repeat (x, width) {
            if (at[y][x] != empty_block) {
                int ny = y;
                while (ny+1 < height + pack_size and at[ny+1][x] == empty_block) ++ ny;
                swap(at[y][x], at[ny][x]);
            }
        }
        // 検査
        int erase_count = 0;
        bool used[height + pack_size][width] = {};
        repeat (y, height + pack_size) repeat (x, width) {
            const int dy[] = { 1, 1, 0, -1 };
            const int dx[] = { 0, 1, 1,  1 };
            repeat (d, 4) {
                int acc = 0;
                int l;
                for (l = 0; acc < sum; ++ l) {
                    int ny = y + dy[d] * l;
                    int nx = x + dx[d] * l;
                    if (not is_on_field(ny, nx, height + pack_size, width)) break;
                    if (at[ny][nx] == empty_block) break;
                    acc += at[ny][nx];
                }
                if (acc == sum) {
                    erase_count += l;
                    repeat (i, l) {
                        int ny = y + dy[d] * i;
                        int nx = x + dx[d] * i;
                        used[ny][nx] = true;
                    }
                }
            }
        }
        score += chain_coefficient[chain] * (erase_count / 2);
        if (not erase_count) break;
        // 消滅
        repeat (y, height + pack_size) repeat (x, width) {
            if (used[y][x]) {
                at[y][x] = empty_block;
            }
        }
    }
    // 3. お邪魔ブロックをお邪魔ストックに追加
    // 4. お邪魔ストックの相殺
    // 5. お邪魔ブロックをネクストパックに挿入
    // 6. お邪魔ストックの持ち越し
    // nop
    // 7. ターン終了
    field_t nfield;
    repeat (y, pack_size) repeat (x, width) if (at[y][x] != empty_block) throw simulate_pack_gameover_exception();
    repeat (y,    height) repeat (x, width) nfield.at[y][x] = at[y + pack_size][x];
    // return
    simulate_pack_result_t result;
    result.score = score;
    result.field = nfield;
    return result;
}

class AI {
private:
    config_t config;
    vector<input_t> inputs;
    vector<output_t> outputs;

private:
    default_random_engine engine;
    double random() {
        uniform_real_distribution<double> dist;
        return dist(engine);
    }
    int randint(int l, int r) { // [l, r]
        uniform_int_distribution<int> dist(l, r);
        return dist(engine);
    }

public:
    AI(config_t const & a_config) {
        engine = default_random_engine(); // fixed seed
        config = a_config;
    }
    output_t think(input_t const & input) {
        // prepare
        field_t const & field = input.self_field;
        pack_t const & pack = config.packs[input.current_turn];

        // check
        if (not inputs.empty()) {
            auto & last = inputs.back();
            simulate_pack_result_t result = simulate_pack(last.self_field, config.packs[last.current_turn], last.self_obstacles, outputs.back());
            assert (result.field == field);
        }

        // do
        output_t output = {};
        int score = -2;
        double priority = 0;
        field_t next_field = field;
        auto use = [&](int x, rotate_t r, simulate_pack_result_t const & result, double p) {
            output = make_output(x, r);
            score = result.score;
            priority = p;
            next_field = result.field;
        };
        int limit = 10; {
            assert (10 < height);
            repeat (y, 10) {
                if (whole(count, field.at[height-y-1], empty_block) <= 2) {
                    limit -= 1;
                }
            }
        }
        repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
            try {
                simulate_pack_result_t result = simulate_pack(field, pack, input.self_obstacles, make_output(x, r));
                if (1 <= result.score and result.score <= limit) result.score = -1;
                double p = random();
                if (score == -1) use(x, r, result, p);
                if (make_pair(score, priority) < make_pair(result.score, p)) use(x, r, result, p);
            } catch (simulate_pack_invalid_output_exception e) {
                // nop
            } catch (simulate_pack_gameover_exception e) {
                // nop
            }
        }

        // log
        cerr << endl;
        cerr << "turn: " << input.current_turn << endl;
        cerr << "limit: " << limit << endl;
        cerr << "score: " << score << endl;
        cerr << next_field << endl;

        // finalize
        inputs.push_back(input);
        outputs.push_back(output);
        return output;
    }
};

const string ai_name = "SampleAI.cpp";
int main() {
    cout << ai_name << endl;
    cout.flush();
    config_t config; cin >> config;
    AI ai(config);
    repeat (i, (int)config.packs.size()) {
        input_t input; cin >> input;
        if (not cin) { cerr << "error: input failed" << endl; break; }
        output_t output = ai.think(input);
        cout << output << endl;
        cout.flush();
    }
    return 0;
}
