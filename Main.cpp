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
#include <chrono>
#include <cmath>
#include <cassert>
#define repeat(i,n) for (int i = 0; (i) < (n); ++(i))
#define repeat_from(i,m,n) for (int i = (m); (i) < (n); ++(i))
#define repeat_reverse(i,n) for (int i = (n)-1; (i) >= 0; --(i))
#define repeat_from_reverse(i,m,n) for (int i = (n)-1; (i) >= (m); --(i))
#define whole(f,x,...) ([&](decltype((x)) whole) { return (f)(begin(whole), end(whole), ## __VA_ARGS__); })(x)
typedef long long ll;
using namespace std;
template <class T> void setmax(T & a, T const & b) { if (a < b) a = b; }
template <class T> void setmin(T & a, T const & b) { if (b < a) a = b; }
bool is_on_field(int y, int x, int h, int w) { return 0 <= y and y < h and 0 <= x and x < w; }

struct point_t { int y, x; };
point_t point(int y, int x) { return (point_t) { y, x }; }
point_t operator + (point_t const & a, point_t const & b) { return point(a.y + b.y, a.x + b.x); }
point_t operator - (point_t const & a, point_t const & b) { return point(a.y - b.y, a.x - b.x); }
point_t operator * (int a, point_t const & b) { return point(a * b.y, a * b.x); }
point_t & operator += (point_t & a, point_t const & b) { a.y += b.y; a.x += b.x; return a; }
point_t & operator -= (point_t & a, point_t const & b) { a.y -= b.y; a.x -= b.x; return a; }
template <typename T> point_t point(T const & p) { return (point_t) { p.y, p.x }; }
bool operator == (point_t a, point_t b) { return make_pair(a.y, a.x) == make_pair(b.y, b.x); }
bool operator != (point_t a, point_t b) { return make_pair(a.y, a.x) != make_pair(b.y, b.x); }
bool operator <  (point_t a, point_t b) { return make_pair(a.y, a.x) <  make_pair(b.y, b.x); }

namespace primitive {
    const int pack_size = 3;
    const int width = 10;
    const int height = 16;
    const int erasing_sum = 10;
    const int turn_number = 500;

    typedef char block_t;
    const block_t empty_block = 0;
    const block_t obstacle_block = erasing_sum + 1;
    template <size_t H, size_t W>
    struct blocks_t {
        array<array<block_t, H>, W> at; // 左下が原点, x座標が先
    };
    typedef blocks_t<pack_size, pack_size> pack_t;
    typedef blocks_t<height, width> field_t;
    template <size_t H, size_t W>
    istream & operator >> (istream & in, blocks_t<H, W> & a) {
        repeat_reverse (y, H) {
            repeat (x, W) {
                int c; in >> c; a.at[x][y] = c;
                assert ((a.at[x][y] == 0 and 0 == empty_block)
                        or (1 <= a.at[x][y] and a.at[x][y] <= 9)
                        or (a.at[x][y] == 11 and 11 == obstacle_block));
            }
        }
        string s; in >> s; assert (s == "END");
        return in;
    }
    template <size_t H, size_t W>
    ostream & operator << (ostream & out, blocks_t<H, W> const & a) {
        repeat_reverse (y, H) {
            if (y != H-1) out << endl;
            repeat (x, W) {
                switch (a.at[x][y]) {
                    case empty_block: out << ' '; break;
                    case obstacle_block: out << '#'; break;
                    default: out << int(a.at[x][y]); break;
                }
            }
        }
        return out;
    }
    template <size_t H, size_t W> bool operator == (blocks_t<H, W> const & a, blocks_t<H, W> const & b) { return a.at == b.at; }
    template <size_t H, size_t W> bool operator != (blocks_t<H, W> const & a, blocks_t<H, W> const & b) { return a.at != b.at; }
    template <size_t H, size_t W> bool operator <  (blocks_t<H, W> const & a, blocks_t<H, W> const & b) { return a.at <  b.at; }

    const int dangerline = height + 1;
    struct config_t {
        // int width, height;
        // int pack_size, erasing_sum;
        vector<pack_t> packs;
    };
    istream & operator >> (istream & in, config_t & a) {
        int w, h, t, s, n; in >> w >> h >> t >> s >> n;
        assert (w == width);
        assert (h == height);
        assert (t == pack_size);
        assert (s == erasing_sum);
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
    ostream & operator << (ostream & out, output_t const & a) { return out << a.x << ' ' << a.rotate; }
    output_t make_output(int x, rotate_t rotate) { return { x, rotate }; }
    bool operator < (output_t const & a, output_t const & b) { return make_pair(a.x, a.rotate) < make_pair(b.x, b.rotate); }
}
using namespace primitive;

pack_t rotate(pack_t a, rotate_t r) {
    pack_t b;
    switch (r % 4 + (r < 0 ? 4 : 0)) {
        case 0: b = a; break;
        case 1: repeat (x, pack_size) repeat (y, pack_size) b.at[x][y] = a.at[2-y][  x]; break;
        case 2: repeat (x, pack_size) repeat (y, pack_size) b.at[x][y] = a.at[2-x][2-y]; break;
        case 3: repeat (x, pack_size) repeat (y, pack_size) b.at[x][y] = a.at[  y][2-x]; break;
    }
    return b;
}

pack_t fill_obstacles(pack_t a, int obstacles) {
    repeat_reverse (y, pack_size) { // yが先
        repeat (x, pack_size) {
            if (obstacles and a.at[x][y] == empty_block) {
                a.at[x][y] = obstacle_block;
                obstacles -= 1;
            }
        }
    }
    return a;
}

int count_empty_blocks(pack_t const & a) {
    int cnt = 0;
    repeat (x, pack_size) repeat (y, pack_size) if (a.at[x][y] == empty_block) ++ cnt;
    return cnt;
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
    repeat (dy, pack_size) repeat (dx, pack_size) if (pack.at[dx][dy] != empty_block) {
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
        while (y < H and field.at[x][y] != empty_block) ++ y;
        h[x] = y;
    }
    return h;
}
vector<point_t> drop_pack(blocks_t<height + pack_size, width> & field, array<int,width> & height_map, pack_t const & a_pack, output_t const & output) {
    vector<point_t> modified_blocks;
    pack_t pack = rotate(a_pack, output.rotate); // お邪魔ブロックは既に置かれているとする
    repeat (dy, 3) repeat (dx, 3) {
        if (pack.at[dx][dy] != empty_block) {
            int nx = output.x + dx;
            assert (0 <= nx and nx < width);
            field.at[nx][height_map[nx]] = pack.at[dx][dy];
            if (field.at[nx][height_map[nx]] != obstacle_block) {
                modified_blocks.push_back(point(height_map[nx], nx));
            }
            ++ height_map[nx];
        }
    }
    return modified_blocks;
}
template<size_t H, size_t W>
vector<pair<point_t,int> > collect_erases(blocks_t<H,W> const & field, vector<point_t> const & modified_blocks) {
    static const int dy[] = { -1, -1, 0, 1 }; // 下 右下 右 右上
    static const int dx[] = {  0,  1, 1, 1 };
    vector<pair<point_t,int> > erases;
    for (point_t p : modified_blocks) { // 変化したところだけ見る
        assert (field.at[p.x][p.y] != empty_block and field.at[p.x][p.y] != obstacle_block);
        repeat (j, 4) {
            int cnt = 1;
            int acc = field.at[p.x][p.y];
            int ly = p.y - dy[j];
            int lx = p.x - dx[j];
            for (; is_on_field(ly, lx, H, W); ++ cnt) {
                block_t block = field.at[lx][ly];
                if (block == empty_block) break;
                if (acc + block > erasing_sum) break;
                acc += block;
                ly -= dy[j];
                lx -= dx[j];
            }
            // しゃくとり法っぽく
            int ry = p.y + dy[j];
            int rx = p.x + dx[j];
            while (cnt --) {
                while (is_on_field(ry, rx, H, W)) {
                    block_t block = field.at[rx][ry];
                    if (block == empty_block) break;
                    if (acc + block > erasing_sum) break;
                    acc += block;
                    ry += dy[j];
                    rx += dx[j];
                }
                ly += dy[j];
                lx += dx[j];
                if (acc == erasing_sum) erases.emplace_back(point(ly, lx), j);
                acc -= field.at[lx][ly];
            }
        }
    }
    whole(sort, erases);
    erases.erase(whole(unique, erases), erases.end()); // 重複排除
    return erases;
}
template<size_t H, size_t W>
pair<int, vector<point_t> > apply_erases(blocks_t<H,W> const & field, vector<pair<point_t,int> > const & erases) {
    static const int dy[] = { -1, -1, 0, 1 }; // 下 右下 右 右上
    static const int dx[] = {  0,  1, 1, 1 };
    int erase_count = 0;
    vector<point_t> used;
    for (auto && it : erases) {
        point_t p; int j; tie(p, j) = it;
        int cnt = 0, acc = 0;
        for (; acc != erasing_sum; ++ cnt) {
            acc += field.at[p.x][p.y];
            used.push_back(p);
            p.y += dy[j];
            p.x += dx[j];
        }
        erase_count += cnt;
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
        at[p.x][p.y] = empty_block;
        setmin(height_map[p.x], p.y);
    }
    vector<point_t> modified_blocks;
    repeat (x,W) {
        for (int y = height_map[x] + 1; y < old_height_map[x]; ++ y) {
            if (at[x][y] != empty_block) {
                at[x][height_map[x]] = at[x][y];
                if (at[x][height_map[x]] != obstacle_block) {
                    modified_blocks.push_back(point(height_map[x], x));
                }
                ++ height_map[x];
                at[x][y] = empty_block;
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
    repeat (x, width) repeat (y,    height) workspace.at[x][y] = field.at[x][y];
    repeat (x, width) repeat (y, pack_size) workspace.at[x][height + y] = empty_block;
    // 1. パックの投下
    array<int,width> height_map = make_height_map(field);
    vector<point_t> modified_blocks = drop_pack(workspace, height_map, pack, output);
    // simulate()
    simulate_result_t result = simulate(workspace, height_map, modified_blocks, 0);
    // result
    repeat (x, width) if (height_map[x] > height) throw simulate_gameover_exception();
    field_t nfield;
    repeat (x, width) repeat (y, height) nfield.at[x][y] = workspace.at[x][y];
    return { result, nfield };
}

simulate_result_t estimate_with_erasing(field_t const & field) {
    const array<int,width> height_map = make_height_map(field);
    simulate_result_t acc = { -1, 0 };
    repeat (x, width) {
        repeat_reverse (y, height_map[x]) {
            if (field.at[x][y] == empty_block or field.at[x][y] == obstacle_block) continue;
            bool erasable =
                y+1 >= height
                or (x-1 >= 0    and field.at[x-1][y+1] == empty_block)
                or (                field.at[x  ][y+1] == empty_block)
                or (x+1 < width and field.at[x+1][y+1] == empty_block);
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

simulate_result_t estimate_with_drop(field_t const & field) {
    const array<int,width> height_map = make_height_map(field);
    simulate_result_t acc = { -1, 0 };
    repeat (x, width) {
        repeat_from (b,1,9+1) {
            blocks_t<height + 1, width> nfield = {};
            repeat (nx, width) repeat (y, height_map[nx]) nfield.at[nx][y] = field.at[nx][y];
            nfield.at[x][height_map[x]] = b;
            array<int,width> nheight_map = height_map;
            nheight_map[x] += 1;
            vector<point_t> modified_blocks { point(height_map[x], x) };
            simulate_result_t result = simulate(nfield, nheight_map, modified_blocks, 0);
            setmax(acc, result);
        }
    }
    return acc;
}

const int summary_depth = 12;
struct state_summary_t {
    int base_turn;
    // そのターンのパックのみを落として検査する
    simulate_result_t result[summary_depth];
    simulate_result_t estimated[summary_depth + 1];
    simulate_result_t best_result[summary_depth]; // そのターン以降で
    simulate_result_t best_estimated[summary_depth + 1];
};
state_summary_t summarize_state(config_t const & config, int current_turn, field_t const & field, int a_obstacles) {
    state_summary_t info = {};
    info.base_turn = current_turn;
    info.estimated[0] = estimate_with_erasing(field);
    int obstacles = a_obstacles;
    repeat (age, summary_depth) {
        if (current_turn + age >= config.packs.size()) break;
        pack_t pack = fill_obstacles(config.packs[current_turn + age], min(9, obstacles));
        obstacles -= min(9, obstacles);
        repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
            simulate_result_t result; field_t nfield;
            try {
                tie(result, nfield) = simulate_with_output(field, pack, make_output(x, r));
            } catch (simulate_invalid_output_exception e) {
                continue;
            } catch (simulate_gameover_exception e) {
                continue;
            }
            setmax(info.result[age], result);
            setmax(info.estimated[age], estimate_with_erasing(nfield));
        }
    }
    info.best_result   [summary_depth  -1] = info.result   [summary_depth  -1];
    info.best_estimated[summary_depth+1-1] = info.estimated[summary_depth+1-1];
    repeat_reverse (age, summary_depth   - 1) info.best_result   [age] = max(info.best_result   [age+1], info.result   [age]);
    repeat_reverse (age, summary_depth+1 - 1) info.best_estimated[age] = max(info.best_estimated[age+1], info.estimated[age]);
    return info;
}

struct field_ptr_t {
    array<uint64_t,2> hash;
    shared_ptr<field_t> field;
};
field_ptr_t new_field_ptr(field_t const & field) {
    field_ptr_t ptr;
    string s(height * width, '\0');
    string t(height * width, '\0');
    repeat (x, width) repeat (y, height) {
        if (field.at[x][y] == empty_block) break;
        s[x * width + y] = field.at[x][y];
        t[x * width + y] = field.at[x][y] << 4;
    }
    ptr.hash[0] = hash<string>()(s);
    ptr.hash[1] = hash<string>()(t);
    ptr.field = make_shared<field_t>(field);
    return ptr;
}
field_ptr_t null_field_ptr() {
    field_ptr_t ptr = {};
    return ptr;
}
bool operator < (field_ptr_t const & a, field_ptr_t const & b) {
    if (a.hash != b.hash) return a.hash < b.hash; // 辞書順
    if (a.field == b.field) return false;
    return *a.field < *b.field;
}

struct photon_t {
    int age;
    field_ptr_t ptr;
    int estimated_chain;
    int best_estimated_chain;
    int score;
    int obstacles; // smaller is better
    bool effective_fired;
    double evaluated_value;
    double permanent_bonus;
    output_t output;
};
bool operator < (photon_t const & a, photon_t const & b) {
    return a.evaluated_value < b.evaluated_value; // weak
}

photon_t initial_photon(input_t const & input, int last_score) {
    photon_t pho;
    pho.age = 0;
    pho.ptr = new_field_ptr(input.self_field);
    pho.estimated_chain = -1;
    pho.best_estimated_chain = -1;
    pho.score = last_score;
    pho.obstacles = input.self_obstacles - input.opponent_obstacles;
    pho.effective_fired = false;
    pho.output = make_output(0xdeadbeef, 0);
    pho.evaluated_value = 0;
    pho.permanent_bonus = 0;
    return pho;
}

const int chain_of_fire = 6; // 発火したとみなすべき連鎖数 (inclusive)
double evaluate_photon_permanent_bonus(photon_t const & pho, simulate_result_t const & result, simulate_result_t const & estimated, state_summary_t const & oppo_sum, photon_t const & previous_pho) {
    double acc = 0;
    if (result.chain >= chain_of_fire and result.chain == previous_pho.best_estimated_chain - 1) acc -= 100;
    if (result.chain >= chain_of_fire and result.chain == previous_pho.best_estimated_chain - 2) acc -= 200;
    if (result.chain >= chain_of_fire and result.chain <= previous_pho.best_estimated_chain - 3) acc -= 300;
    if (pho.effective_fired and not previous_pho.effective_fired) {
        acc += 300 + 0.2 * result.score; // 発火可能でしかも勝てるというのは大きい
    }
    return acc; // 差分
}
double evaluate_photon(photon_t const & pho, simulate_result_t const & result, simulate_result_t const & estimated, state_summary_t const & oppo_sum, photon_t const & previous_pho) {
    double acc = 0;
    acc += pho.score; // scoreを基準に
    acc += estimated.chain * 20; // 大連鎖だと4*5なんて誤差、小連鎖でscoreを気にされると不利
    acc += (1 - 0.08 * pho.age) * estimated.score; // 不正確な値だけど比較可能だろうからよい
    acc -= 3 * max(0, min(160, pho.obstacles - estimated.score / 6));
    repeat (x, width) repeat (y, height) {
        int dx = min(x, width-x-1);
        if (pho.ptr.field->at[x][y] == obstacle_block) {
            acc -= 3 + 0.3 * y + 1.2 * dx; // 端に寄せた方がいいけど、あまりそればかり気にされても困る
        }
    }
    if (result.chain <= 2) acc -= 3.0 * result.score; // 手数で損
    if (result.chain == 3) acc -= 2.0 * result.score;
    if (result.chain == 4) acc -= 1.5 * result.score;
    if (result.chain == 5) acc -= 1.0 * result.score;
    return acc + pho.permanent_bonus;
}
bool is_effective_firing(int score, int obstacles, int age, state_summary_t const & oppo_sum) {
    assert (4 < summary_depth);
    int i = min(4, age);
    return max<int>(oppo_sum.best_result[i].score, oppo_sum.best_estimated[i].score * 0.8) + max(obstacles + 20, 60) * 5 < 1.2 * score;
}

struct update_photon_exception {};
const int memo_limit = 5;
photon_t update_photon(photon_t const & previous_pho, pack_t const & pack, output_t output, state_summary_t const & oppo_sum,
        map<tuple<field_ptr_t, int, output_t>, pair<simulate_result_t, field_ptr_t> > & sim_memo,
        map<field_ptr_t, simulate_result_t> & est_memo) { // throws exceptions
    photon_t npho = previous_pho;
    int used_obstacles = max(0, min(count_empty_blocks(pack), npho.obstacles));
    npho.obstacles -= used_obstacles;
    simulate_result_t result;
    auto sim_key = make_tuple(npho.ptr, used_obstacles, output);
    if (sim_memo.count(sim_key)) {
        tie(result, npho.ptr) = sim_memo[sim_key]; // errorなら登録されてない (いっそ落とした方が速い)
    } else {
        try {
            field_t nfield;
            tie(result, nfield) = simulate_with_output(*npho.ptr.field, fill_obstacles(pack, used_obstacles), output);
            npho.ptr = new_field_ptr(nfield);
            if (result.chain >= memo_limit) sim_memo[sim_key] = { result, npho.ptr };
        } catch (simulate_invalid_output_exception e) {
            throw update_photon_exception();
        } catch (simulate_gameover_exception e) {
            throw update_photon_exception();
        }
    }
    npho.obstacles -= count_obstacles_from_delta(npho.score, result.score);
    npho.score += result.score;
    if (not est_memo.count(npho.ptr)) est_memo[npho.ptr] = estimate_with_erasing(*npho.ptr.field);
    simulate_result_t estimated = est_memo[npho.ptr];
    npho.estimated_chain = estimated.chain;
    if (is_effective_firing(result.score, previous_pho.obstacles, npho.age, oppo_sum)) npho.effective_fired = true;
    if (result.chain >= chain_of_fire) npho.best_estimated_chain = -1; // 発火したら初期化
    setmax(npho.best_estimated_chain, estimated.chain);
    npho.evaluated_value += evaluate_photon_permanent_bonus(npho, result, estimated, oppo_sum, previous_pho);
    npho.evaluated_value = evaluate_photon(npho, result, estimated, oppo_sum, previous_pho);
    if (npho.age == 0) npho.output = output;
    npho.age += 1;
    return npho;
}

class AI {
private:
    config_t config;
    vector<input_t> inputs;
    vector<output_t> outputs;
    vector<int> scores;
    vector<simulate_result_t> results;

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

private:
    static const int beam_small_width = 3;
    static const int beam_chain_max = 40;
    static const int beam_depth_max = 12;
    static constexpr double beam_output_limit_rate = 0.3;
    array<map<tuple<field_ptr_t, int, output_t>, pair<simulate_result_t, field_ptr_t> >, beam_depth_max> sim_memo;
    array<map<field_ptr_t, simulate_result_t>, beam_depth_max> est_memo;
    vector<simulate_result_t> estimated_history;

public:
    AI(config_t const & a_config) {
        engine = default_random_engine(); // fixed seed
        config = a_config;
    }
    output_t think(input_t const & input) {
        // prepare
        int turn = input.current_turn;
        field_t const & field = input.self_field;
        pack_t const & pack = config.packs[turn];
        pack_t const & filled_pack = fill_obstacles(pack, input.self_obstacles);
        const int last_score = scores.empty() ? 0 : scores.back();

        // logging
        cerr << endl;
        cerr << "turn: " << turn << endl;
        cerr << "remaining time: " << input.remaining_time << endl;
        if (not scores.empty()) cerr << "score: " << scores.back() << endl;
        cerr << "obstacles: " << input.self_obstacles - input.opponent_obstacles << endl;
        cerr << "self estimated chain: " << estimate_with_erasing(input.self_field).chain << endl;
        cerr << "opp. estimated chain: " << estimate_with_erasing(input.opponent_field).chain << endl;

        // check
        if (not inputs.empty()) {
            auto & last = inputs.back();
            pack_t const & last_filled_pack = fill_obstacles(config.packs[last.current_turn], last.self_obstacles);
            simulate_result_t result; field_t nfield; tie(result, nfield) = simulate_with_output(last.self_field, last_filled_pack, outputs.back());
            if (nfield != field) {
                cerr << "<<<" << endl;
                cerr << nfield << endl;
                cerr << "===" << endl;
                cerr << field << endl;
                cerr << ">>>" << endl;
            }
            assert (nfield == field);
        }

        output_t output = make_output(0xdeadbeef, 0);

        // look at opponent
        const state_summary_t oppo_sum = summarize_state(config, turn, input.opponent_field, input.opponent_obstacles); {
            int best_score = 0;
            repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
                simulate_result_t result;
                try {
                    result = simulate_with_output(field, filled_pack, output).first;
                } catch (simulate_invalid_output_exception e) {
                    continue;
                } catch (simulate_gameover_exception e) {
                    continue;
                }
                if (is_effective_firing(result.score, input.self_obstacles, 0, oppo_sum)) {
                    if (best_score < result.score) {
                        best_score = result.score;
                        output = make_output(x, r);
                    }
                }
            }
        }

        // estimation
        int estimated_chain = 0; {
            simulate_result_t estimated = estimate_with_erasing(input.self_field);
            setmax(estimated_chain, estimated.chain);
            repeat_from (i,1,6) {
                if (turn-i <= 0) break;
                if (results[turn-i].chain >= chain_of_fire) break;
                setmax(estimated_chain, estimated_history[turn-i].chain);
            }
            estimated_history.push_back(estimated); // scopeを切ったのでその場で追加までする
        }

        // beam search
        int beam_width =
            input.remaining_time < 20 * 1000 ?  30 :
            input.remaining_time < 30 * 1000 ?  80 :
            input.remaining_time < 40 * 1000 ? 160 :
            input.remaining_time < 50 * 1000 ? 180 :
            input.remaining_time < 60 * 1000 ? 200 :
            250;
        int beam_depth =
            estimated_chain <  4 ? 12 :
            estimated_chain <  6 ? 10 :
            estimated_chain <  8 ? 9 :
            8;
        beam_depth = round(beam_depth * (
            input.remaining_time < 20 * 1000 ? 0.7 :
            input.remaining_time < 40 * 1000 ? 0.8 :
            input.remaining_time < 60 * 1000 ? 0.9 :
            1.0));
        assert (beam_depth <= beam_depth_max);
        auto memo_ix_at = [&](int age) { return (turn + age) % beam_depth_max; };
        sim_memo[memo_ix_at(beam_depth - 1)].clear();
        est_memo[memo_ix_at(beam_depth - 1)].clear();
        if (turn > 0) {
            input_t const & last = inputs.back();
            int estimated_obstacles = max(0, last.self_obstacles - count_empty_blocks(config.packs[turn-1]));
            if (estimated_obstacles < input.self_obstacles) {
                int actual_obstacles = input.self_obstacles;
                bool diff = false;
                repeat (age, beam_depth-1) {
                    if (config.packs.size() <= turn + age) break;
                    int de = max(0, min(count_empty_blocks(config.packs[turn + age]), estimated_obstacles));
                    int da = max(0, min(count_empty_blocks(config.packs[turn + age]), actual_obstacles));
                    if (de != da) diff = true;
                    estimated_obstacles -= de;
                    actual_obstacles    -= da;
                    if (diff) {
                        sim_memo[memo_ix_at(age)].clear();
                        est_memo[memo_ix_at(age)].clear();
                    }
                }
            }
        }
        if (output.x == 0xdeadbeef) {
            vector<photon_t> beam;
            array<vector<photon_t>, beam_chain_max> nbeam = {};
            beam.push_back(initial_photon(input, last_score));
            beam.back().best_estimated_chain = estimated_chain;
            repeat (age, beam_depth) {
                int memo_ix = memo_ix_at(age);
                if (turn + age >= config.packs.size()) break; // game ends
                for (photon_t const & pho : beam) {
                    repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
                        try {
                            photon_t npho = update_photon(pho, config.packs[turn + pho.age], make_output(x, r), oppo_sum, sim_memo[memo_ix], est_memo[memo_ix]);
                            npho.evaluated_value += random() * 0.1; // tiebreak
                            nbeam[npho.estimated_chain].push_back(npho);
                        } catch (update_photon_exception e) {
                            continue;
                        }
                    }
                }
                beam.clear();
                vector<pair<int,int> > ix1;
                repeat (i, nbeam.size()) {
                    int w = min<int>(beam_small_width, nbeam[i].size());
                    vector<int> ix2(nbeam[i].size()); whole(iota, ix2, 0);
                    partial_sort(ix2.begin(), ix2.begin() + w, ix2.end(), [&](int j1, int j2) {
                        return nbeam[i][j2] < nbeam[i][j1]; // reversed
                    });
                    repeat (j,w) beam.push_back(nbeam[i][ix2[j]]);
                    repeat_from (j,w,min<int>(beam_width,nbeam[i].size())) ix1.emplace_back(i, ix2[j]);
                }
                int w = max(0, min<int>(beam_width - beam.size(), ix1.size()));
                whole(sort, ix1, [&](pair<int,int> i, pair<int,int> j) {
                    return nbeam[j.first][j.second] < nbeam[i.first][i.second]; // reversed
                });
                map<output_t, int> cnt;
                repeat (i,w) {
                    photon_t const & pho = nbeam[ix1[i].first][ix1[i].second];
                    if (cnt[pho.output] < w * beam_output_limit_rate) {
                        cnt[pho.output] += 1;
                        beam.push_back(pho);
                    }
                }
                repeat (i,nbeam.size()) nbeam[i].clear();
                if (not beam.empty()) {
                    photon_t & pho = *whole(max_element, beam);
                    output = pho.output;
                    if (age == beam_depth-1) {
                        simulate_result_t result = estimate_with_erasing(*pho.ptr.field);
                        cerr << "beam " << age << " width: " << beam.size() << endl;
                        cerr << "    evaluated: " << pho.evaluated_value << endl;
                        cerr << "    chain: + " << result.chain << endl;
                        cerr << "    score: " << pho.score << " + " << result.score << endl;
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
            results.push_back(simulate_with_output(field, filled_pack, output).first);
            scores.push_back(last_score + results.back().score);
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
