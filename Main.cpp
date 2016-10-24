#ifndef LOCAL
#pragma GCC target "tune=native"
#pragma GCC optimize "O3,omit-frame-pointer,inline"
#endif
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
#define whole(f,x,...) ([&](auto && whole) { return (f)(begin(whole), end(whole), ## __VA_ARGS__); })(x)
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
    template <size_t H, size_t W>
    struct blocks_t {
        array<array<block_t, W>, H> at; // 左下が原点
    };
    typedef blocks_t<pack_size, pack_size> pack_t;
    typedef blocks_t<height, width> field_t;
    template <size_t H, size_t W>
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
    template <size_t H, size_t W>
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
    template <size_t H, size_t W> bool operator == (blocks_t<H, W> const & a, blocks_t<H, W> const & b) { return a.at == b.at; }
    template <size_t H, size_t W> bool operator != (blocks_t<H, W> const & a, blocks_t<H, W> const & b) { return a.at != b.at; }

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
int calculate_score(int chain, int erase_count) {
    return chain_coefficient[chain] * (erase_count / 2);
}
int count_obstacles_from_delta(int base, int delta) {
    return (base + delta) / 5 - base / 5;
}

bool is_valid_output(field_t const & field, pack_t const & a_pack, output_t const & output) {
    if (0 <= output.x and output.x + pack_size <= width) return true;
    pack_t pack = rotate(a_pack, output.rotate); // お邪魔ブロックは既に置かれているとする
    repeat (dy, pack_size) repeat (dx, pack_size) if (pack.at[dy][dx] != empty_block) {
        int x = output.x + dx;
        if (x < 0 or width <= x) return false;
    }
    return true;
}

template<size_t H, size_t W>
array<int,W> make_height_map(blocks_t<H,W> const & field) {
    array<int,W> h;
    repeat (x,W) {
        int y = 0;
        while (y < H and field.at[y][x] != empty_block) ++ y;
        h[x] = y;
    }
    return h;
}
vector<point_t> drop_pack(blocks_t<height + pack_size, width> & field, array<int,width> & height_map, pack_t const & a_pack, output_t const & output) {
    vector<point_t> modified_blocks;
    pack_t pack = rotate(a_pack, output.rotate); // お邪魔ブロックは既に置かれているとする
    repeat (dy, 3) repeat (dx, 3) {
        if (pack.at[dy][dx] != empty_block) {
            int nx = output.x + dx;
            assert (0 <= nx and nx < width);
            field.at[height_map[nx]][nx] = pack.at[dy][dx];
            if (field.at[height_map[nx]][nx] != obstacle_block) {
                modified_blocks.push_back(point(height_map[nx], nx));
            }
            ++ height_map[nx];
        }
    }
    return modified_blocks;
}
point_t simulate_point_from(point_t const & p, int d, int i) {
    const int dy[] = { -1, -1, 0, 1 }; // 下 右下 右 右上
    const int dx[] = {  0,  1, 1, 1 };
    int ny = p.y + dy[d] * i;
    int nx = p.x + dx[d] * i;
    return point(ny, nx);
}
template<size_t H, size_t W>
block_t simulate_at_from(blocks_t<H,W> const & field, point_t const & q, int d, int i) {
    point_t p = simulate_point_from(q, d, i);
    if (not is_on_field(p.y, p.x, H, W)) return obstacle_block;
    if (field.at[p.y][p.x] == empty_block) return obstacle_block;
    return field.at[p.y][p.x];
}
template<size_t H, size_t W>
vector<pair<point_t,int> > collect_erases(blocks_t<H,W> & field, vector<point_t> const & modified_blocks) {
    vector<pair<point_t,int> > erases;
    for (point_t p : modified_blocks) { // 変化したところだけ見る
        assert (field.at[p.y][p.x] != empty_block and field.at[p.y][p.x] != obstacle_block);
        repeat (d, 4) {
            auto point1 = [&](int i) { return simulate_point_from(p, d, i); };
            auto    at1 = [&](int i) { return simulate_at_from<H,W>(field, p, d, i); };
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
    return erases;
}
template<size_t H, size_t W>
pair<int, vector<point_t> > apply_erases(blocks_t<H,W> const & field, vector<pair<point_t,int> > const & erases) {
    int erase_count = 0;
    vector<point_t> used;
    for (auto it : erases) {
        point_t p; int d; tie(p, d) = it;
        auto point1 = [&](int i) { return simulate_point_from(p, d, i); };
        auto    at1 = [&](int i) { return simulate_at_from<H,W>(field, p, d, i); };
        int l = 0, acc = 0;
        for (; acc < sum; ++ l) {
            acc += at1(l);
            used.push_back(point1(l));
        }
        erase_count += l;
    }
    whole(sort, used);
    used.erase(whole(unique, used), used.end());
    return { erase_count, used };
}
template<size_t H, size_t W>
vector<point_t> erase_blocks(blocks_t<H,W> & field, array<int,W> & height_map, vector<point_t> const & points_to_erase) {
    auto & at = field.at;
    array<int,W> old_height_map = height_map;
    for (point_t p : points_to_erase) {
        at[p.y][p.x] = empty_block;
        setmin(height_map[p.x], p.y);
    }
    vector<point_t> modified_blocks;
    repeat (x,W) {
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
    return modified_blocks;
}

struct simulate_result_t {
    int score;
    int chain;
};
bool operator < (simulate_result_t const & a, simulate_result_t const & b) {
    return make_pair(a.score, a.chain) < make_pair(b.score, b.chain);
}

template<size_t H, size_t W>
simulate_result_t simulate(blocks_t<H,W> & field, array<int,W> & height_map, vector<point_t> modified_blocks, int initial_chain) {
    // 2. ブロックの消滅&落下処理
    int score = 0;
    int chain = initial_chain;
    while (not modified_blocks.empty()) {
        // 検査
        vector<pair<point_t,int> > erases = collect_erases(field, modified_blocks);
        if (erases.empty()) break; // 消滅なし
        ++ chain; // ここでincrement
        // 消滅&落下
        int erase_count; tie(erase_count, modified_blocks) = apply_erases(field, erases);
        modified_blocks = erase_blocks(field, height_map, modified_blocks);
        score += calculate_score(chain, erase_count);
    }
    // 3. 4. 5. 6. お邪魔ブロック関連処理
    // nop
    // 7. ターン終了
    simulate_result_t result;
    result.score = score;
    result.chain = chain;
    return result;
}
struct simulate_invalid_output_exception {};
struct simulate_gameover_exception {};
pair<simulate_result_t, field_t> simulate_with_output(field_t const & field, pack_t const & pack, output_t const & output) { // throws exceptions
    if (not is_valid_output(field, pack, output)) throw simulate_invalid_output_exception();
    blocks_t<height + pack_size, width> workspace;
    repeat (y,    height) repeat (x, width) workspace.at[y][x] = field.at[y][x];
    repeat (y, pack_size) repeat (x, width) workspace.at[height + y][x] = empty_block;
    // 1. パックの投下
    array<int,width> height_map = make_height_map(field);
    vector<point_t> modified_blocks = drop_pack(workspace, height_map, pack, output);
    // simulate()
    simulate_result_t result = simulate(workspace, height_map, modified_blocks, 0);
    // result
    repeat (x, width) if (height_map[x] > height) throw simulate_gameover_exception();
    field_t nfield;
    repeat (y, height) repeat (x, width) nfield.at[y][x] = workspace.at[y][x];
    return { result, nfield };
}

simulate_result_t estimate_chain(field_t const & field) {
    const array<int,width> height_map = make_height_map(field);
    simulate_result_t acc = { -1, 0 };
    repeat (x, width) {
        repeat_reverse (y, height_map[x]) {
            if (field.at[y][x] == empty_block or field.at[y][x] == obstacle_block) continue;
            bool erasable =
                y+1 >= height
                or (x-1 >= 0    and field.at[y+1][x-1] == empty_block)
                or (                field.at[y+1][x  ] == empty_block)
                or (x+1 < width and field.at[y+1][x+1] == empty_block);
            if (not erasable) break;
            field_t nfield = field;
            array<int,width> nheight_map = height_map;
            vector<point_t> modified_blocks { point(y, x) };
            modified_blocks = erase_blocks(nfield, nheight_map, modified_blocks);
            simulate_result_t result = simulate(nfield, nheight_map, modified_blocks, 1);
            if (make_pair(acc.chain, acc.score) < make_pair(result.chain, result.score)) {
                acc = result;
            }
        }
    }
    return acc;
}

const int opponent_depth = 8;
struct opponent_info_t {
    int base_turn;
    // そのターンのパックのみを落として検査する
    simulate_result_t result[opponent_depth];
    simulate_result_t estimated[opponent_depth + 1];
    simulate_result_t best_result;
    simulate_result_t best_estimated;
};
opponent_info_t estimate_opponent(config_t const & config, input_t const & input) {
    opponent_info_t info = {};
    info.base_turn = input.current_turn;
    info.estimated[0] = estimate_chain(input.opponent_field);
    int obstacles = input.opponent_obstacles;
    repeat (age, opponent_depth) {
        if (input.current_turn + age >= config.packs.size()) break;
        pack_t pack = fill_obstacles(config.packs[input.current_turn + age], min(9, obstacles));
        obstacles -= min(9, obstacles);
        repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
            simulate_result_t result; field_t nfield;
            try {
                tie(result, nfield) = simulate_with_output(input.opponent_field, pack, make_output(x, r));
            } catch (simulate_invalid_output_exception e) {
                continue;
            } catch (simulate_gameover_exception e) {
                continue;
            }
            setmax(info.result[age], result);
            setmax(info.estimated[age], estimate_chain(nfield));
        }
    }
    repeat (age, opponent_depth) setmax(info.best_result, info.result[age]);
    repeat (age, opponent_depth + 1) setmax(info.best_estimated, info.estimated[age]);
    return info;
}

struct photon_t {
    int age;
    field_t field;
    int score;
    int obstacles; // smaller is better
    double evaluated_value;
    output_t output;
};
bool operator < (photon_t const & a, photon_t const & b) {
    return a.evaluated_value < b.evaluated_value; // weak
}

photon_t initial_photon(input_t const & input, int last_score) {
    photon_t pho;
    pho.age = 0;
    pho.field = input.self_field;
    pho.score = last_score;
    pho.obstacles = input.self_obstacles - input.opponent_obstacles;
    pho.output = make_output(0xdeadbeef, 0);
    return pho;
}
double evaluate_photon(photon_t const & pho, simulate_result_t const & result, opponent_info_t const & oppo) {
    double acc = 0;
    acc += pho.score; // scoreを基準に
    simulate_result_t estimated = estimate_chain(pho.field);
    acc += 10 * estimated.chain;
    acc += estimated.score; // 不正確な値だけど比較可能だろうからよい
    if (pho.obstacles > 0) acc -= 3 * min(18, pho.obstacles); // 一度降ると消せないので正負に敏感
    // 端によせるべき
    repeat (y, height) repeat (x, width) {
        int lx = min(x, width-x-1);
        if (pho.field.at[y][x] == obstacle_block) {
            acc += - 3 - 0.3 * y - 0.3 * lx;
        } else {
            acc += 2 - 0.3 * y - 0.2 * lx;
        }
    }
    // お邪魔の下に普通ブロックはよくない
    repeat (x, width) {
        int cnt = 0;
        repeat (y, height) {
            if (pho.field.at[y][x] == empty_block) {
                break;
            } else if (pho.field.at[y][x] == obstacle_block) {
                acc -= 0.1 * min(4, cnt);
            } else {
                cnt += 1;
            }
        }
    }
    // でこぼこさせない
    auto height_map = make_height_map(pho.field);
    acc -= 0.8 * pow(abs(height - height_map[0]), 1.3);
    acc -= 0.6 * pow(abs(height_map[0] - height_map[1]), 1.3);
    repeat_from (x, 1, width-2) acc -= 0.4 * (abs(height_map[x] - height_map[x+1]));
    acc -= 0.6 * pow(abs(height_map[width-2] - height_map[width-1]), 1.3);
    acc -= 0.8 * pow(abs(height_map[width-1] - height), 1.3);
    // 下手な消しはすべきでない 手数で損
    switch (result.chain) {
        case 0: // throughout
        case 1: // throughout
        case 2: acc -= 2.0 * result.score; break;
        case 3: acc -= 1.3 * result.score; break;
        case 4: acc -= 0.8 * result.score; break;
        case 5: acc -= 0.5 * result.score; break;
        case 6: acc -= 0.3 * result.score; break;
    }
    if (result.chain == 4) acc -= 0.3 * result.score;
    if (result.chain == 5) acc -= 0.5 * result.score;
    if (result.chain == 6) acc -= 0.7 * result.score;
    int score_delta = result.score - max<int>(oppo.best_result.score * 1.3, oppo.best_estimated.score * 0.7);
    if (result.chain >= 7) {
        if      (score_delta < - 100 * 5) acc -= 0.8 * result.score;
        else if (score_delta < -  20 * 5) acc -= 0.6 * result.score;
        else if (score_delta < -  10 * 5) acc -= 0.4 * result.score;
        else if (score_delta <     0 * 5) acc -= 0.2 * result.score;
        else if (score_delta <    10 * 5) acc -= 0.1 * result.score;
        else if (score_delta <    20 * 5) acc += 0.0 * result.score;
        else if (score_delta <    30 * 5) acc += 0.1 * result.score + 400 * (1 - 0.1 * pho.age);
        else if (score_delta <    40 * 5) acc += 0.2 * result.score + 400 * (1 - 0.1 * pho.age);
        else if (score_delta <    50 * 5) acc += 0.3 * result.score + 400 * (1 - 0.1 * pho.age);
        else if (score_delta <    60 * 5) acc += 0.4 * result.score + 400 * (1 - 0.1 * pho.age);
        else if (score_delta <    80 * 5) acc += 0.5 * result.score + 400 * (1 - 0.1 * pho.age);
        else if (score_delta <   100 * 5) acc += 0.6 * result.score + 400 * (1 - 0.1 * pho.age);
        else if (score_delta <   110 * 5) acc += 0.7 * result.score + 400 * (1 - 0.1 * pho.age);
        else if (score_delta <   120 * 5) acc += 0.8 * result.score + 400 * (1 - 0.1 * pho.age);
        else if (score_delta <   130 * 5) acc += 0.9 * result.score + 400 * (1 - 0.1 * pho.age);
        else if (score_delta <   140 * 5) acc += 1.0 * result.score + 400 * (1 - 0.1 * pho.age);
        else if (score_delta <   150 * 5) acc += 1.1 * result.score + 400 * (1 - 0.1 * pho.age);
        else                              acc += 1.2 * result.score + 400 * (1 - 0.1 * pho.age);
    }
    return acc;
}
struct update_photon_exception {};
photon_t update_photon(photon_t const & previous_pho, pack_t const & pack, output_t output, opponent_info_t const & oppo) { // throws exceptions
    photon_t npho = previous_pho;
    npho.age += 1;
    int used_obstacles = max(0, min(9, npho.obstacles));
    npho.obstacles -= used_obstacles;
    simulate_result_t result;
    try {
        tie(result, npho.field) = simulate_with_output(npho.field, fill_obstacles(pack, used_obstacles), output);
    } catch (simulate_invalid_output_exception e) {
        throw update_photon_exception();
    } catch (simulate_gameover_exception e) {
        throw update_photon_exception();
    }
    npho.obstacles -= count_obstacles_from_delta(npho.score, result.score);
    npho.score += result.score;
    npho.evaluated_value = evaluate_photon(npho, result, oppo);
    if (previous_pho.age == 0) npho.output = output;
    return npho;
}

class AI {
private:
    config_t config;
    vector<input_t> inputs;
    vector<output_t> outputs;
    vector<int> scores;

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
        const int last_score = scores.empty() ? 0 : scores.back();

        // logging
        cerr << endl;
        cerr << "turn: " << input.current_turn << endl;
        cerr << "remaining time: " << input.remaining_time << endl;
        if (not scores.empty()) cerr << "score: " << scores.back() << endl;
        cerr << "obstacles: " << input.self_obstacles - input.opponent_obstacles << endl;
        cerr << "self estimated chain: " << estimate_chain(input.self_field).chain << endl;
        cerr << "opp. estimated chain: " << estimate_chain(input.opponent_field).chain << endl;

        // check
#ifndef NDEBUG
        if (not inputs.empty()) {
            auto & last = inputs.back();
            pack_t const & last_filled_pack = fill_obstacles(config.packs[last.current_turn], last.self_obstacles);
            simulate_result_t result; field_t nfield; tie(result, nfield) = simulate_with_output(last.self_field, last_filled_pack, outputs.back());
            if (result.field != field) {
                cerr << "<<<" << endl;
                cerr << result.field << endl;
                cerr << "===" << endl;
                cerr << field << endl;
                cerr << ">>>" << endl;
            }
            assert (result.field == field);
        }
#endif

        // opponent
        opponent_info_t oppo = estimate_opponent(config, input);
        cerr << "oppo best result    chain : " << oppo.best_result.chain << endl;
        cerr << "oppo best estimated chain : " << oppo.best_estimated.chain << endl;

        // beam search
        output_t output = make_output(0xdeadbeef, 0); {
            const int beam_width = 200;
            const int beam_depth = 6;
            vector<photon_t> beam, nbeam;
            beam.push_back(initial_photon(input, last_score));
            repeat (age, beam_depth) {
                if (input.current_turn + age >= config.packs.size()) break; // game ends
                for (photon_t const & pho : beam) {
                    repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
                        photon_t npho;
                        try {
                            npho = update_photon(pho, config.packs[input.current_turn + pho.age], make_output(x, r), oppo);
                            npho.evaluated_value += random() * 0.1; // tiebreak
                            nbeam.push_back(npho);
                        } catch (update_photon_exception e) {
                            continue;
                        }
                    }
                }
                vector<int> ix(nbeam.size()); whole(iota, ix, 0);
                int w = min<int>(beam_width, nbeam.size());
                partial_sort(ix.begin(), ix.begin() + w, ix.end(), [&](int i, int j) {
                    return nbeam[j] < nbeam[i]; // reversed
                });
                beam.resize(w);
                repeat (i,w) beam[i] = nbeam[ix[i]];
                nbeam.clear();
                if (not beam.empty()) {
                    output = beam.front().output;
                    if (age == beam_depth-1) {
                        simulate_result_t result = estimate_chain(beam.front().field);
                        cerr << "beam " << age << " width: " << beam.size() << endl;
                        cerr << "    score: " << beam.front().score << endl;
                        cerr << "    estimated chain: " << result.chain << endl;
                        cerr << "    estimated score: " << result.score << endl;
                    }
                }
            }
        }

        // finalize
        if (not is_valid_output(field, filled_pack, output)) {
            repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
                try {
                    simulate_with_output(field, filled_pack, output);
                    assert (false);
                } catch (simulate_invalid_output_exception e) {
                    // nop
                } catch (simulate_gameover_exception e) {
                    // nop
                }
            }
            cerr << "lose..." << endl;
            output = make_output(0, 0);
        } else {
            inputs.push_back(input);
            outputs.push_back(output);
            scores.push_back(simulate_with_output(field, filled_pack, output).first.score);
        }
        return output;
    }
};

const string ai_name = "SampleAI.cpp";
int main() {
    cout << ai_name << endl;
    cout.flush();
    config_t config; cin >> config;
    AI ai(config);
    repeat (i, config.packs.size()) {
        input_t input; cin >> input;
        if (not cin) { cerr << "error: input failed" << endl; break; }
        output_t output = ai.think(input);
        cout << output << endl;
        cout.flush();
    }
    return 0;
}
