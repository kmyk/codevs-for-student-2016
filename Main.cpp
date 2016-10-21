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
        repeat_reverse (y, H) {
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
        repeat_reverse (y, H) {
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
    template <int H, int W> bool operator == (blocks_t<H, W> const & a, blocks_t<H, W> const & b) { return a.at == b.at; }
    template <int H, int W> bool operator != (blocks_t<H, W> const & a, blocks_t<H, W> const & b) { return a.at != b.at; }

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
    pack_t b;
    switch (r % 4 + (r < 0 ? 4 : 0)) {
        case 0: b = a; break;
        case 1: repeat (y, pack_size) repeat (x, pack_size) b.at[y][x] = a.at[  x][2-y]; break;
        case 2: repeat (y, pack_size) repeat (x, pack_size) b.at[y][x] = a.at[2-y][2-x]; break;
        case 3: repeat (y, pack_size) repeat (x, pack_size) b.at[y][x] = a.at[2-x][  y]; break;
    }
    return b;
}

pack_t fill_obstacles(pack_t a, int obstacles) {
    repeat_reverse (y, pack_size) {
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

bool is_valid_output(field_t const & field, pack_t const & a_pack, output_t const & output) {
    if (0 <= output.x and output.x + pack_size <= width) return true;
    pack_t pack = rotate(a_pack, output.rotate); // お邪魔ブロックは既に置かれているとする
    repeat (dy, pack_size) repeat (dx, pack_size) if (pack.at[dy][dx] != empty_block) {
        int x = output.x + dx;
        if (x < 0 or width <= x) return false;
    }
    return true;
}

array<int,width> make_height_map(field_t const & field) {
    array<int,width> h;
    repeat (x, width) {
        int y = 0;
        while (y < height and field.at[y][x] != empty_block) ++ y;
        h[x] = y;
    }
    return h;
}

struct simulate_pack_gameover_exception {};
struct simulate_pack_result_t {
    int score;
    field_t field;
};
simulate_pack_result_t simulate_pack(field_t const & field, array<int,width> const & a_height_map, pack_t const & a_pack, output_t const & output) { // throws exceptions
    assert (is_valid_output(field, a_pack, output));
    blocks_t<height + pack_size, width> workspace;
    auto & at = workspace.at;
    array<int,width> height_map = a_height_map;
    repeat (y,    height) repeat (x, width) at[y][x] = field.at[y][x];
    repeat (y, pack_size) repeat (x, width) at[height + y][x] = empty_block;
    // 1. パックの投下
    vector<point_t> modified_blocks;
    pack_t pack = rotate(a_pack, output.rotate); // お邪魔ブロックは既に置かれているとする
    repeat (dy, 3) repeat (dx, 3) {
        if (pack.at[dy][dx] != empty_block) {
            int nx = output.x + dx;
            assert (0 <= nx and nx < width);
            at[height_map[nx]][nx] = pack.at[dy][dx];
            if (at[height_map[nx]][nx] != obstacle_block) {
                modified_blocks.push_back(point(height_map[nx], nx));
            }
            ++ height_map[nx];
        }
    }
    // 2. ブロックの消滅&落下処理
    const int dy[] = { -1, -1, 0, 1 }; // 下 右下 右 右上
    const int dx[] = {  0,  1, 1, 1 };
    auto point_from = [&](point_t const & p, int d, int i) {
        int ny = p.y + dy[d] * i;
        int nx = p.x + dx[d] * i;
        return point(ny, nx);
    };
    auto at_from = [&](point_t const & q, int d, int i) {
        point_t p = point_from(q, d, i);
        if (not is_on_field(p.y, p.x, height + pack_size, width)) return obstacle_block;
        if (at[p.y][p.x] == empty_block) return obstacle_block;
        return at[p.y][p.x];
    };
    int score = 0;
    int chain = 1;
    for (; not modified_blocks.empty(); ++ chain) {
        // 検査
        vector<pair<point_t,int> > erases;
        for (point_t p : modified_blocks) { // 変化したところだけ見る
            assert (at[p.y][p.x] != empty_block and at[p.y][p.x] != obstacle_block);
            repeat (d, 4) {
                auto point1 = [&](int i) { return point_from(p, d, i); };
                auto    at1 = [&](int i) { return    at_from(p, d, i); };
                int l = 0, acc = at1(l);
                while (acc + at1(l - 1) <= sum) {
                    -- l;
                    acc += at1(l);
                }
                // しゃくとり法
                int r = 1; // [l, r)
                while (l <= 0) {
                    while (acc + at1(r) <= sum) {
                        acc += at1(r);
                        ++ r;
                    }
                    if (acc == sum) erases.emplace_back(point1(l), d);
                    acc -= at1(l);
                    ++ l;
                }
            }
        }
        whole(sort, erases);
        erases.erase(whole(unique, erases), erases.end()); // 重複排除
        // 消滅&落下
        int erase_count = 0;
        vector<point_t> used;
        for (auto it : erases) {
            point_t p; int d; tie(p, d) = it;
            auto point1 = [&](int i) { return point_from(p, d, i); };
            auto    at1 = [&](int i) { return    at_from(p, d, i); };
            int l = 0, acc = 0;
            for (; acc < sum; ++ l) {
                acc += at1(l);
                used.push_back(point1(l));
            }
            erase_count += l;
        }
        array<int,width> old_height_map = height_map;
        for (point_t p : used) {
            at[p.y][p.x] = empty_block;
            setmin(height_map[p.x], p.y);
        }
        modified_blocks.clear();
        repeat (x,width) {
            for (int y = height_map[x] + 1; y < old_height_map[x]; ++ y) {
                if (at[y][x] != empty_block) {
                    at[height_map[x]][x] = at[y][x];
                    if (at[height_map[x]][x] != obstacle_block) {
                        modified_blocks.push_back(point(height_map[x], x));
                    }
                    ++ height_map[x];
                    at[y][x] = empty_block;
                }
            }
        }
        score += chain_coefficient[chain] * (erase_count / 2);
    }
    // 3. 4. 5. 6. お邪魔ブロック関連処理
    // nop
    // 7. ターン終了
    field_t nfield;
    repeat (y, pack_size) repeat (x, width) if (at[height + y][x] != empty_block) throw simulate_pack_gameover_exception();
    repeat (y,    height) repeat (x, width) nfield.at[y][x] = at[y][x];
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
        pack_t const & filled_pack = fill_obstacles(pack, input.self_obstacles);
        const array<int,width> height_map = make_height_map(field);

        // check
        if (not inputs.empty()) {
            auto & last = inputs.back();
            pack_t const & last_filled_pack = fill_obstacles(config.packs[last.current_turn], last.self_obstacles);
            simulate_pack_result_t result = simulate_pack(last.self_field, make_height_map(last.self_field), last_filled_pack, outputs.back());
            if (result.field != field) {
                cerr << "<<<" << endl;
                cerr << result.field << endl;
                cerr << "===" << endl;
                cerr << field << endl;
                cerr << ">>>" << endl;
            }
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
            if (not is_valid_output(field, filled_pack, make_output(x, r))) continue;
            try {
                simulate_pack_result_t result = simulate_pack(field, height_map, filled_pack, make_output(x, r));
                if (1 <= result.score and result.score <= limit) result.score = -1;
                double p = random();
                if (score == -1) use(x, r, result, p);
                if (make_pair(score, priority) < make_pair(result.score, p)) use(x, r, result, p);
            } catch (simulate_pack_gameover_exception e) {
                // nop
            }
        }

        // log
        cerr << endl;
        cerr << "turn: " << input.current_turn << endl;
        cerr << "remaining time: " << input.remaining_time << endl;
        cerr << "limit: " << limit << endl;
        cerr << "score: " << score << endl;
        // cerr << next_field << endl;

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
