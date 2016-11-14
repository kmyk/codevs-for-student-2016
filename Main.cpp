#ifdef RELEASE
#ifndef NDEBUG
#define NDEBUG
#endif
#endif
#include <iostream>
#include <vector>
#include <algorithm>
#include <array>
#include <set>
#include <map>
#include <deque>
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

template <typename T>
using functional_priority_queue = priority_queue<T, vector<T>, function<bool (T const &, T const &)> >;

struct clock_check {
    chrono::high_resolution_clock::time_point clock_end;
    clock_check(ll msec) {
        chrono::high_resolution_clock::time_point clock_begin = chrono::high_resolution_clock::now();
        clock_end = clock_begin + chrono::duration<ll, milli>(msec);
    }
    bool operator() () const {
        chrono::high_resolution_clock::time_point clock_current = chrono::high_resolution_clock::now();
        return clock_current < clock_end;
    }
};

struct stopwatch {
    chrono::high_resolution_clock::time_point clock_begin;
    stopwatch() {
        clock_begin = chrono::high_resolution_clock::now();
    }
    ll operator() () const {
        chrono::high_resolution_clock::time_point clock_end = chrono::high_resolution_clock::now();
        return chrono::duration_cast<chrono::milliseconds>(clock_end - clock_begin).count();
    }
};

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
    template<size_t H, size_t W>
    int count_block(blocks_t<H,W> const & a, block_t b) {
        int cnt = 0;
        repeat (x, W) repeat (y, H) if (a.at[x][y] == b) ++ cnt;
        return cnt;
    }
    int count_obstacle_blocks(field_t const & a) {
        return count_block(a, obstacle_block);
    }

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
        int turn;
        int remaining_time; // in msec
        int self_obstacles;
        field_t self_field;
        int opponent_obstacles;
        field_t opponent_field;
    };
    istream & operator >> (istream & in, input_t & a) {
        in >> a.turn >> a.remaining_time;
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
    bool operator  < (output_t const & a, output_t const & b) { return make_pair(a.x, a.rotate)  < make_pair(b.x, b.rotate); }
    bool operator == (output_t const & a, output_t const & b) { return make_pair(a.x, a.rotate) == make_pair(b.x, b.rotate); }
    bool operator != (output_t const & a, output_t const & b) { return make_pair(a.x, a.rotate) != make_pair(b.x, b.rotate); }
    const output_t invalid_output = { (int)0xdeadbeef, -1 }; // 簡単のため範囲を拡張

    pack_t rotate(pack_t a, rotate_t r) {
        assert (pack_size == 3);
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
        return count_block(a, empty_block);
    }
    int count_consumed_obstacles(pack_t const & pack, int obstacles) {
        return max(0, min(count_empty_blocks(pack), obstacles));
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
}
using namespace primitive;

namespace simulation {
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

    struct result_t {
        int score;
        int chain;
    };
    bool compare_result_with_score(result_t const & a, result_t const & b) { return make_pair(a.score, a.chain) < make_pair(b.score, b.chain); }
    bool compare_result_with_chain(result_t const & a, result_t const & b) { return make_pair(a.chain, a.score) < make_pair(b.chain, b.score); }
    bool compare_result_with_score_reversed(result_t const & a, result_t const & b) { return make_pair(a.score, a.chain) > make_pair(b.score, b.chain); }
    bool compare_result_with_chain_reversed(result_t const & a, result_t const & b) { return make_pair(a.chain, a.score) > make_pair(b.chain, b.score); }
    bool operator < (result_t const & a, result_t const & b) { return compare_result_with_score(a, b); }

    template<size_t H, size_t W>
    result_t simulate(blocks_t<H,W> & field, array<int,W> & height_map, vector<point_t> modified_blocks, int initial_chain) {
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
        result_t result;
        result.score = score;
        result.chain = chain;
        return result;
    }
    struct simulate_invalid_output_exception {};
    struct simulate_gameover_exception {};
    tuple<result_t, int, field_t> simulate_with_output(field_t const & field, pack_t const & pack, output_t const & output) { // throws exceptions
        if (not is_valid_output(field, pack, output)) throw simulate_invalid_output_exception();
        blocks_t<height + pack_size, width> workspace;
        repeat (x, width) repeat (y,    height) workspace.at[x][y] = field.at[x][y];
        repeat (x, width) repeat (y, pack_size) workspace.at[x][height + y] = empty_block;
        // 1. パックの投下
        array<int,width> height_map = make_height_map(field);
        vector<point_t> modified_blocks = drop_pack(workspace, height_map, pack, output);
        const array<int,width> height_map_original = height_map;
        // simulate()
        result_t result = simulate(workspace, height_map, modified_blocks, 0);
        // result
        repeat (x, width) if (height_map[x] > height) throw simulate_gameover_exception();
        int result_erased = 0; repeat (x, width) result_erased += height_map_original[x] - height_map[x];
        field_t nfield; repeat (x, width) repeat (y, height) nfield.at[x][y] = workspace.at[x][y];
        return make_tuple(result, result_erased, nfield);
    }

    vector<pair<result_t, int> > estimate_with_erasing_all(field_t const & field) {
        const array<int,width> height_map = make_height_map(field);
        vector<pair<result_t, int> > acc;
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
                result_t result = simulate(nfield, nheight_map, modified_blocks, 1);
                int consumed = 0;
                repeat (x, width) consumed += height_map[x] - nheight_map[x];
                acc.emplace_back(result, consumed);
            }
        }
        if (acc.empty()) acc.emplace_back((result_t) { 0, 0 }, 0);
        return acc;
    }
    result_t estimate_with_erasing(field_t const & field) {
        vector<pair<result_t, int> > acc = estimate_with_erasing_all(field);
        return whole(max_element, acc, [&](pair<result_t, int> const & a, pair<result_t, int> const & b) {
            return make_tuple(a.first.chain, - a.second, a.first.score) < make_tuple(b.first.chain, - b.second, b.first.score);
        })->first;
    }

    result_t estimate_with_drop(field_t const & field) {
        const array<int,width> height_map = make_height_map(field);
        result_t acc = { -1, 0 };
        repeat (x, width) {
            repeat_from (b,1,9+1) {
                blocks_t<height + 1, width> nfield = {};
                repeat (nx, width) repeat (y, height_map[nx]) nfield.at[nx][y] = field.at[nx][y];
                nfield.at[x][height_map[x]] = b;
                array<int,width> nheight_map = height_map;
                nheight_map[x] += 1;
                vector<point_t> modified_blocks { point(height_map[x], x) };
                result_t result = simulate(nfield, nheight_map, modified_blocks, 0);
                setmax(acc, result);
            }
        }
        return acc;
    }
}
using namespace simulation;

struct evaluateion_info_t {
    double score;
    double permanent_bonus;
    vector<pair<result_t, int> > estimateds;
};
struct photon_t {
    int turn;
    int score;
    int obstacles; // 負なら相手に送る分 / 相手が発火したらここが変わり次以降の状態が消去される
    int dropped_obstacles;
    field_t field;
    output_t output; // この手に至る辺
    result_t result; // その結果
    int result_erased;
    evaluateion_info_t evaluation;
    weak_ptr<photon_t> parent; // 逆辺
    shared_ptr<array<array<shared_ptr<photon_t>, 4>, 12> > next; // 次状態への辺
};

shared_ptr<photon_t> initial_photon(int turn, int obstacles, field_t const & field) {
    shared_ptr<photon_t> pho = make_shared<photon_t>();
    pho->turn = turn;
    pho->score = 0;
    pho->obstacles = obstacles;
    pho->dropped_obstacles = count_obstacle_blocks(field);
    pho->field = field;
    pho->output = invalid_output;
    pho->result = {};
    pho->result_erased = 0;
    pho->parent.reset();
    pho->next = nullptr;
    // evaluation
    pho->evaluation.score = 0;
    pho->evaluation.permanent_bonus = 0;
    pho->evaluation.estimateds.emplace_back((result_t) { 0, 0 }, 0);
    return pho;
}

// 評価 諸々を気にせず純粋に盤面のみから
void evaluate_photon_for_search(shared_ptr<photon_t> const & pho) {
    vector<pair<result_t, int> > estimateds = estimate_with_erasing_all(pho->field);
    pho->evaluation.estimateds.resize(min<int>(3, estimateds.size()));
    partial_sort_copy(estimateds.begin(), estimateds.end(), pho->evaluation.estimateds.begin(), pho->evaluation.estimateds.end(), [&](pair<result_t, int> const & a, pair<result_t, int> const & b) {
        return make_tuple(a.first.score, a.first.chain, - a.second) > make_tuple(b.first.score, b.first.chain, - b.second); // reversed
    });
    double acc = 0;
    acc += pho->score; // scoreを基準に
    repeat (i, pho->evaluation.estimateds.size()) {
        double k1 = i == 0 ? 1.0 : 0.2;
        acc += k1 * pho->evaluation.estimateds[i].first.score; // 不正確な値だけど比較可能だろうからよい
        acc += k1 * pho->evaluation.estimateds[i].first.chain * 40; // 大連鎖だと誤差、小連鎖でscoreを気にされると不利
        acc -= k1 * pho->evaluation.estimateds[i].second * 4;
    }
    acc -= 21 * pho->result_erased; // 無駄に消すべきでない
    pho->evaluation.score = acc + pho->evaluation.permanent_bonus;
}

// 次のstepを(まだなら)作成 評価もする
void step_photon(shared_ptr<photon_t> const & pho, pack_t const & pack) {
    if (pho->next) return;
    int consumed = count_consumed_obstacles(pack, pho->obstacles);
    pack_t filled_pack = fill_obstacles(pack, consumed);
    pho->next = make_unique<array<array<shared_ptr<photon_t>, 4>, 12> >();
    repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
        shared_ptr<photon_t> npho = make_shared<photon_t>();
        npho->output = make_output(x, r);
        try {
            tie(npho->result, npho->result_erased, npho->field) = simulate_with_output(pho->field, filled_pack, npho->output);
        } catch (simulate_invalid_output_exception e) {
            continue;
        } catch (simulate_gameover_exception e) {
            continue;
        }
        npho->turn = pho->turn + 1;
        npho->score = pho->score + npho->result.score;
        npho->obstacles = pho->obstacles - consumed - count_obstacles_from_delta(pho->score, npho->result.score);
        npho->dropped_obstacles = pho->dropped_obstacles + consumed;
        npho->parent = pho;
        (*pho->next)[x+pack_size-1][r] = npho;
        evaluate_photon_for_search(npho);
    }
}

// 相手の発火でお邪魔が増えたとき
template <typename F>
void update_photon_obstacles(shared_ptr<photon_t> const & pho, int updated_obstacles, vector<pack_t> const & packs, F cont) {
    if (pho->obstacles == updated_obstacles) {
        // nop
    } else if (not pho->next) {
        pho->obstacles = updated_obstacles;
        evaluate_photon_for_search(pho);
    } else {
        int         consumed = count_consumed_obstacles(packs[pho->turn],    pho->obstacles);
        int updated_consumed = count_consumed_obstacles(packs[pho->turn], updated_obstacles);
        pho->obstacles = updated_obstacles;
        evaluate_photon_for_search(pho);
        if (consumed != updated_consumed) {
            pho->next = nullptr; // 開放
            cont(pho);
        } else {
            repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
                shared_ptr<photon_t> npho = (*pho->next)[x+pack_size-1][r];
                if (not npho) continue;
                int next_obstacles = pho->obstacles - updated_consumed - count_obstacles_from_delta(pho->score, npho->result.score);
                update_photon_obstacles(npho, next_obstacles, packs, cont);
            }
        }
    }
}

struct opponent_info_t {
    vector<result_t> result;
    result_t best;
    int score;
    int dropped_obstacles;
};

const int chain_of_fire = 6; // 発火したとみなすべき連鎖数 (inclusive)
bool is_effective_firing(int score, int obstacles, int age, opponent_info_t const & oppo) {
    assert (age >= 1);
    return oppo.score + max(obstacles + 20, max(20, 60 - oppo.dropped_obstacles/2)) * 5 < score;
}

void evaluate_photon_init(shared_ptr<photon_t> const & pho) {
    pho->evaluation.score = 0;
    pho->evaluation.permanent_bonus = 0;
}

// 意思決定と探索順序の基準は別
double evaluate_photon_for_output(shared_ptr<photon_t> const & pho, int base_turn, opponent_info_t const & oppo) {
    int age = pho->turn - base_turn;
    double acc = 0;
    if (is_effective_firing(pho->result.score, pho->obstacles, age, oppo)) {
        if (pho->obstacles < 40) {
            double k = pow(1 + 0.18, -age+1);
            acc += 400 + k * (1.2 * pho->result.score); // 発火した それが可能でしかも勝てるというのは大きい
            acc -= 5 * pho->dropped_obstacles;
            acc -= 3 * max(0, pho->obstacles);
        } else {
            acc = pho->result.score; // 負けてる時はもう全力で
        }
    } else {
        if (pho->result.chain >= chain_of_fire) {
            double k = pow(1 + 0.18, -age+1);
            acc += k * pho->result.score;
            acc -= 3 * max(0, min(30, pho->obstacles));
        } else {
            acc += max(0.1, 1 - 0.03 * (age - 1)) * pho->evaluation.score;
        }
    }
    return acc;
}

shared_ptr<photon_t> parent_photon_at(shared_ptr<photon_t> pho, int turn) {
    while (pho and pho->turn > turn) pho = pho->parent.lock();
    if (not pho or pho->turn != turn) return nullptr;
    return pho;
}

void prune_photon(shared_ptr<photon_t> const & pho, output_t output) {
    assert (pho);
    if (not pho->next) return;
    repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
        if (x == output.x and r == output.rotate) continue;
        (*pho->next)[x+pack_size-1][r] = nullptr;
    }
}

bool compare_photon(shared_ptr<photon_t> const & a, shared_ptr<photon_t> const & b) { return a->evaluation.score < b->evaluation.score; }
bool compare_photon_reversed(shared_ptr<photon_t> const & a, shared_ptr<photon_t> const & b) { return a->evaluation.score > b->evaluation.score; }
bool compare_photon_with_score(shared_ptr<photon_t> const & a, shared_ptr<photon_t> const & b) { return a->result.score < b->result.score; }
bool compare_photon_with_score_reversed(shared_ptr<photon_t> const & a, shared_ptr<photon_t> const & b) { return a->result.score > b->result.score; }
bool compare_photon_with_first(pair<double, weak_ptr<photon_t> > const & a, pair<double, weak_ptr<photon_t> > const & b) { return a.first < b.first; }
bool compare_photon_with_first_reversed(pair<double, weak_ptr<photon_t> > const & a, pair<double, weak_ptr<photon_t> > const & b) { return a.first > b.first; }

template <typename F>
void beam_search(shared_ptr<photon_t> const & initial, config_t const & config, int beam_width, int beam_depth, F cont) {
    vector<shared_ptr<photon_t> > beam, nbeam;
    beam.push_back(initial);
    repeat (i, beam_depth) {
        if (initial->turn + i >= config.packs.size()) break;
        for (shared_ptr<photon_t> const & pho : beam) {
            step_photon(pho, config.packs[initial->turn + i]);
            repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
                shared_ptr<photon_t> npho = (*pho->next)[x+pack_size-1][r];
                if (not npho) continue;
                npho = cont(npho);
                if (not npho) continue;
                if (i+1 < beam_depth) {
                    nbeam.push_back(npho);
                }
            }
        }
        beam.resize(min<int>(beam_width, nbeam.size()));
        partial_sort_copy(nbeam.begin(), nbeam.end(), beam.begin(), beam.end(), compare_photon_reversed);
        nbeam.clear();
    }
}

template <typename F>
void chokudai_search(deque<functional_priority_queue<pair<double, weak_ptr<photon_t> > > > & que, config_t const & config, int initial_turn, int beam_width, int beam_depth, ll time_limit_msec, F cont) {
    assert (initial_turn + beam_depth < config.packs.size());
    assert (beam_depth + 1 < que.size());
    clock_check check(time_limit_msec);
    int begin = 0;
    while (check() and begin < beam_depth) {
        repeat_from (i, begin, beam_depth) {
            repeat (j, beam_width) {
                if (que[i].empty()) {
                    if (i == begin) ++ begin;
                    break;
                }
                shared_ptr<photon_t> pho = que[i].top().second.lock(); que[i].pop();
                if (not pho) { -- j; continue; } // 無視して同じ深さをもう一度
                step_photon(pho, config.packs[initial_turn + i]);
                repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
                    shared_ptr<photon_t> npho = (*pho->next)[x+pack_size-1][r];
                    if (not npho) continue;
                    npho = cont(npho);
                    if (not npho) continue;
                    que[i+1].emplace(npho->evaluation.score, npho);
                }
            }
        }
    }
}

template <typename F>
void chokudai_search(shared_ptr<photon_t> const & initial, config_t const & config, int beam_width, int beam_depth, ll time_limit_msec, F cont) {
    deque<functional_priority_queue<pair<double, weak_ptr<photon_t> > > > que;
    repeat (i, beam_depth + 1) que.emplace_back(compare_photon_with_first);
    que[0].emplace(initial->evaluation.score, initial);
    chokudai_search(que, config, initial->turn, beam_width, beam_depth, time_limit_msec, cont);
}

struct counter_exception {}; // 刺し判定の大域脱出用
class AI {
private:
    config_t config;
    vector<input_t> inputs;
    vector<output_t> outputs;
    vector<int> scores;
    vector<result_t> results;

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
    static const int beam_chain_max = 32;
    static constexpr double beam_output_limit_rate = 0.3;
    vector<shared_ptr<photon_t> > self_history;
    vector<shared_ptr<photon_t> > oppo_history;
    deque<functional_priority_queue<pair<double, weak_ptr<photon_t> > > > que;
    deque<functional_priority_queue<pair<double, weak_ptr<photon_t> > > > fired;

public:
    AI(config_t const & a_config) {
        engine = default_random_engine(); // fixed seed
        config = a_config;
    }
    output_t think(input_t const & input) {
#ifndef RELEASE
        stopwatch watch, total_watch;
#endif

#ifndef RELEASE
        // logging
        cerr << endl;
        cerr << "turn: " << input.turn << endl;
        cerr << "remaining time: " << input.remaining_time << endl;
        cerr << "score: " << (scores.empty() ? 0 : scores.back()) << endl;
        cerr << "obstacles: " << input.self_obstacles - input.opponent_obstacles << endl;
        cerr << "self dropped obstacles: " << count_obstacle_blocks(input.self_field) << endl;
        cerr << "opp. dropped obstacles: " << count_obstacle_blocks(input.opponent_field) << endl;
        cerr << "self estimated chain: " << estimate_with_erasing(input.self_field).chain << endl;
        cerr << "opp. estimated chain: " << estimate_with_erasing(input.opponent_field).chain << endl;

        // check
        if (not inputs.empty()) {
            field_t const & field = input.self_field;
            auto & last = inputs.back();
            pack_t const & last_filled_pack = fill_obstacles(config.packs[last.turn], last.self_obstacles);
            result_t result; int result_erased; field_t nfield; tie(result, result_erased, nfield) = simulate_with_output(last.self_field, last_filled_pack, outputs.back());
            if (nfield != field) {
                cerr << "<<<" << endl;
                cerr << nfield << endl;
                cerr << "===" << endl;
                cerr << field << endl;
                cerr << ">>>" << endl;
            }
            assert (nfield == field);
        }
#endif

        // prepare
        if (input.turn == 0) {
            self_history.push_back(initial_photon(input.turn, input.self_obstacles,     input.self_field));
            oppo_history.push_back(initial_photon(input.turn, input.opponent_obstacles, input.opponent_field));
        }
        { // 自分の
            shared_ptr<photon_t> const & pho = self_history.back();
            vector<shared_ptr<photon_t> > updateds;
            bool updated = (pho->obstacles != input.self_obstacles - input.opponent_obstacles);
            if (updated) {
                if (input.self_obstacles <= 0 and input.self_obstacles - input.opponent_obstacles <= 0) {
                    updated = false; // 勝ってるときは時間節約のため無視
                } else {
                    update_photon_obstacles(pho, input.self_obstacles - input.opponent_obstacles, config.packs, [&](shared_ptr<photon_t> const & pho) {
                        updateds.push_back(pho);
                    });
                }
            }
            evaluate_photon_init(pho);
            step_photon(pho, config.packs[input.turn]);
            // ビームのcacheの更新
            if (input.turn == 0) {
                que.emplace_back(compare_photon_with_first);
                que.back().emplace(pho->evaluation.score, pho);
            } else {
                if (pho->result.chain >= chain_of_fire) {
                    que.clear();
                    fired.clear();
                    que.emplace_back(compare_photon_with_first);
                    que.back().emplace(pho->evaluation.score, pho);
                } else if (updated) {
#ifndef RELEASE
                    cerr << "queue shifted" << endl;
#endif
                    auto shift = [](deque<functional_priority_queue<pair<double, weak_ptr<photon_t> > > > & que) {
                        que[0] = functional_priority_queue<pair<double, weak_ptr<photon_t> > >(compare_photon_with_first);
                        repeat (i, que.size() - 1) {
                            while (not que[i+1].empty()) {
                                shared_ptr<photon_t> npho = que[i+1].top().second.lock(); que[i+1].pop();
                                if (not npho) continue;
                                que[i].emplace(npho->evaluation.score, npho);
                            }
                            que[i+1] = functional_priority_queue<pair<double, weak_ptr<photon_t> > >(compare_photon_with_first);
                        }
                    };
                    shift(que);
                    shift(fired);
                    for (auto & pho : updateds) {
                        int i = pho->turn - input.turn;
                        while (i >= que.size()) que.emplace_back(compare_photon_with_first);
                        que[i].emplace(pho->evaluation.score, pho);
                    }
                } else {
                    que.pop_front();
                    fired.pop_front();
                }
            }
        }
        if (input.turn != 0) { // 相手の
            shared_ptr<photon_t> const & pho = oppo_history.back();
            step_photon(pho, config.packs[input.turn-1]);
            output_t output = invalid_output; // 相手の出力は直接は見えない
            repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
                shared_ptr<photon_t> const & npho = (*pho->next)[x+pack_size-1][r];
                if (not npho) continue;
                if (npho->field == input.opponent_field) {
                    output = npho->output;
                    break;
                }
            }
            if (output == invalid_output) {
#ifndef RELEASE
                cerr << "oppo reset cache" << endl;
#endif
                oppo_history.push_back(initial_photon(input.turn, input.opponent_obstacles - input.self_obstacles, input.opponent_field));
            } else {
                prune_photon(pho, output);
                shared_ptr<photon_t> const & npho = (*pho->next)[output.x+pack_size-1][output.rotate];
                oppo_history.push_back(npho);
                update_photon_obstacles(npho, input.opponent_obstacles - input.self_obstacles, config.packs, [](shared_ptr<photon_t> const & pho){}); // ここでお邪魔を更新
            }
            evaluate_photon_init(oppo_history.back());
            step_photon(oppo_history.back(), config.packs[input.turn]);
        }

        // opponent
#ifndef RELEASE
        watch = stopwatch();
#endif
        const int oppo_depth = max<int>(4, 8 - (180 * 1000 - input.remaining_time) / (30 * 1000));
        opponent_info_t oppo = {}; {
            const int beam_width = 60;
            const int beam_depth = oppo_depth;
            oppo.result.resize(oppo_depth);
            auto cont = [&](shared_ptr<photon_t> const & pho) {
                int age = pho->turn - input.turn;
                assert (age >= 1);
                setmax(oppo.result[age-1], pho->result);
                return pho->result.chain < chain_of_fire ? pho : nullptr; // 打ち切るか否か
            };
            shared_ptr<photon_t> initial = oppo_history.back();
            beam_search(initial, config, beam_width, beam_depth, cont);
        }
        oppo.dropped_obstacles = oppo_history.back()->dropped_obstacles;
        oppo.best = *whole(max_element, oppo.result);
        repeat (i, oppo_depth) setmax<int>(oppo.score, pow(0.8, i) * oppo.result[i].score); // ここけっこう重要
#ifndef RELEASE
        repeat (i, oppo_depth) {
            cerr << "oppo " << i << ": " << oppo.result[i].chain << "c " << oppo.result[i].score << "pt" << endl;
        }
        cerr << "oppo elapsed: " << watch() << "ms" << endl;
#endif

        // chokudai search
#ifndef RELEASE
        watch = stopwatch();
#endif
        output_t output = invalid_output; {
            const int estimated_chain = self_history.back()->evaluation.estimateds.front().first.chain;
            const int beam_width = 3;
            const int beam_depth = max(22 - min(14, input.turn), 18 - estimated_chain/2);
            const int time_limit = min(input.remaining_time / 30, max(1200, 170 * max(estimated_chain, oppo.best.chain))); // msec
            while (beam_depth+1 >= que.size()) que.emplace_back(compare_photon_with_first);
            while (fired.size() < que.size()) fired.emplace_back(compare_photon_with_first);
            bool is_fired = false;
            double best_score = - INFINITY;
            shared_ptr<photon_t> result = nullptr;
            auto cont = [&](shared_ptr<photon_t> const & pho) {
                int age = pho->turn - input.turn;
                assert (age >= 1);
                bool cur_is_fired = is_effective_firing(pho->result.score, pho->obstacles, age, oppo);
                double cur_score = evaluate_photon_for_output(pho, input.turn, oppo);
                if (make_pair(is_fired, best_score) < make_pair(cur_is_fired, cur_score)) {
                    is_fired = cur_is_fired;
                    best_score = cur_score;
                    result = pho;
#ifndef RELEASE
                    cerr << "result: +" << age << "t " << cur_score << "pt";
                    if (pho->result.chain >= chain_of_fire) {
                        bool is_eff = is_effective_firing(pho->result.score, pho->obstacles, age, oppo);
                        cerr << " (fire " << pho->result.chain << "c " << pho->result.score << "pt" << (is_eff ? " eff." : "") << ")";
                    } else {
                        cerr << " (" << pho->evaluation.estimateds.front().first.chain << "c " << pho->evaluation.estimateds.front().first.score << "pt)";
                    }
                    cerr << endl;
#endif
                }
                if (pho->result.chain < chain_of_fire) { // 打ち切るか否か
                    return pho;
                } else {
                    fired[age-1].emplace(pho->result.score, pho);
                    return shared_ptr<photon_t>(nullptr);
                }
            };
#ifndef RELEASE
            cerr << "load cache" << endl;
#endif
            repeat (i, fired.size()) { // 古い葉は覚えておいて直接見る
                while (not fired[i].empty()) {
                    shared_ptr<photon_t> const & pho = fired[i].top().second.lock();
                    if (not pho) { fired[i].pop(); continue; } // だめなのを削る
                    assert (pho->result.chain >= chain_of_fire);
                    cont(pho);
                    break;
                }
            }
#ifndef RELEASE
            cerr << "chokudai search" << endl;
#endif
            chokudai_search(que, config, input.turn, beam_width, beam_depth, time_limit, cont);
            if (result) {
                shared_ptr<photon_t> pho = parent_photon_at(result, input.turn + 1);
                assert (pho);
                output = pho->output;
            }
#ifndef RELEASE
            repeat (i, beam_depth) cerr << "width at " << i << ": beam " << que[i].size() << " / fire " << fired[i].size()<< endl;
            cerr << "done" << endl;
            repeat (i, fired.size()) {
                if (not fired[i].empty()) {
                    shared_ptr<photon_t> const & pho = fired[i].top().second.lock();
                    int age = pho->turn - input.turn;
                    bool is_eff = is_effective_firing(pho->result.score, pho->obstacles, age, oppo);
                    cerr << "fire: +" << age << "t " << pho->result.chain << "c " << pho->result.score << "pt" << (is_eff ? " eff." : "") << endl;
                }
            }
#endif
        }
#ifndef RELEASE
        cerr << "self elapsed: " << watch() << "ms" << endl;
#endif

        // 刺せるなら強制発火
#ifndef RELEASE
        watch = stopwatch();
#endif
        const int fire_depth =
            input.remaining_time > 90 * 1000 ? 3 :
            input.remaining_time > 60 * 1000 ? 2 :
            input.remaining_time > 30 * 1000 ? 1 :
            0;
        if (input.remaining_time > 20 * 1000) {
            if (input.opponent_obstacles <= 5) { // 既に送れてるならしない
                repeat (i, fire_depth) if (i < fired.size() and not fired[i].empty()) {
                    shared_ptr<photon_t> const & best = fired[i].top().second.lock();
                    assert (best);
                    if (output == best->output) break; // もう発火することにしてあるなら見ない
                    if (best->obstacles + max(30, oppo.result[0].score/5 + 10) > 0) continue; // 明らかにだめな場合はしない
#ifndef RELEASE
                    cerr << "firing check for: +" << 1+i << "t " << best->result.chain << "c " << best->result.score << "pt" << endl;
#endif
                    const int beam_width = 120;
                    const int beam_depth = 10;
                    auto cont = [&](shared_ptr<photon_t> pho) {
                        int age = pho->turn - input.turn;
                        assert (age >= 1);
                        if (age == 1) {
                            pho = make_shared<photon_t>(*pho);
                            pho->next = nullptr;
                            pho->obstacles += best->result.score/5; // ここで足す
                        }
                        if (- pho->obstacles >= 20) {
#ifndef RELEASE
                            cerr << "counter found: +" << age << "t " << pho->result.chain << "c " << pho->result.score << "pt" << endl;
#endif
                            throw counter_exception();
                        }
                        return pho->result.chain < chain_of_fire ? pho : nullptr;
                    };
                    shared_ptr<photon_t> const & initial = oppo_history.back();
                    try {
                        beam_search(initial, config, beam_width, beam_depth, cont);
                        output = parent_photon_at(best, input.turn + 1)->output;
#ifndef RELEASE
                        cerr << "fire" << endl;
#endif
                    } catch (counter_exception e) {
                        // nop
                    }
                }
            }
        }
#ifndef RELEASE
        cerr << "firing beam elapsed: " << watch() << "ms" << endl;
#endif

        // finalize
        const pack_t filled_pack = fill_obstacles(config.packs[input.turn], input.self_obstacles);
        if (output == invalid_output) {
#ifndef RELEASE
            cerr << "search failed..." << endl;
#endif
            int score = -1;
            repeat_from (x, - pack_size + 1, width) repeat (r, 4) {
                try {
                    result_t result = get<0>(simulate_with_output(input.self_field, filled_pack, make_output(x, r)));
                    if (score <= result.score) {
                        score = result.score;
                        output = make_output(x, r);
                    }
                } catch (simulate_invalid_output_exception e) {
                    // nop
                } catch (simulate_gameover_exception e) {
                    // nop
                }
            }
#ifndef RELEASE
            if (output == invalid_output) {
                cerr << "greedy failed..." << endl;
            }
#endif
        }
        if (is_valid_output(input.self_field, filled_pack, output)) {
            inputs.push_back(input);
            outputs.push_back(output);
            results.push_back(get<0>(simulate_with_output(input.self_field, filled_pack, output)));
            int last_score = scores.empty() ? 0 : scores.back();
            scores.push_back(last_score + results.back().score);
            prune_photon(self_history.back(), output);
            self_history.push_back((*self_history.back()->next)[output.x+pack_size-1][output.rotate]);
        }
#ifndef RELEASE
        cerr << "total elapsed: " << total_watch() << "ms" << endl;
#endif
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
        if (not cin) {
#ifndef RELEASE
            cerr << "error: input failed" << endl;
#endif
            break;
        }
        output_t output = ai.think(input);
        cout << output << endl;
        cout.flush();
    }
    return 0;
}
