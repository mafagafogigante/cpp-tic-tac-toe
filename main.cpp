#include <algorithm>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

#define debug(x) cout << #x " is " << x << '\n';

class TicTacToe {
  typedef uint8_t PlayerType;
  typedef uint32_t BoardType;
  typedef uint32_t ScoreType;

  const static PlayerType NONE = 0;
  const static PlayerType X = 1;
  const static PlayerType O = 2;

  const static ScoreType WIN = 512;

  const static int TILES = 9;

  BoardType board;
  const bool debugging = false;

  constexpr static int get_shift(const int position) {
    return 2 * (8 - position);
  }

  constexpr static ScoreType decay(const ScoreType score) {
    return 7 * score / 8;
  }

 public:
  constexpr static char get_symbol(const PlayerType player) {
    if (player == O) {
      return 'O';
    } else if (player == X) {
      return 'X';
    } else {
      return '_';
    }
  }

  constexpr static PlayerType get_player(const char symbol) {
    if (symbol == 'O') {
      return O;
    } else if (symbol == 'X') {
      return X;
    } else {
      return NONE;
    }
  }

  TicTacToe(bool debug = false) : board(0), debugging(debug) {
  }

  TicTacToe(string s, bool debug = false) : TicTacToe(debug) {
    for (int i = 0; i < TILES; i++) {
      set(i, get_player(s[i]));
    }
  }

  PlayerType winner() const {
    for (int i = 0; i < 3; i++) {
      int h_base = 3 * i;
      if (get(h_base)) {
        if (get(h_base) == get(h_base + 1) &&
            get(h_base + 1) == get(h_base + 2)) {
          return get(h_base);
        }
      }
      int v_base = i;
      if (get(v_base)) {
        if (get(v_base) == get(v_base + 3) &&
            get(v_base + 3) == get(v_base + 6)) {
          return get(v_base);
        }
      }
    }
    if (get(0)) {
      if (get(0) == get(4) && get(4) == get(8)) {
        return get(0);
      }
    }
    if (get(2)) {
      if (get(2) == get(4) && get(4) == get(6)) {
        return get(2);
      }
    }
    return NONE;
  }

  PlayerType get_player_to_move() const {
    bool x = false;
    for (int i = 0; i < TILES; i++) {
      if (is_free(i)) {
        x = !x;
      }
    }
    return x ? X : O;
  }

  bool winnable() {
    const PlayerType to_move = get_player_to_move();
    for (int i = 0; i < TILES; i++) {
      if (is_free(i)) {
        set(i, to_move);
        bool won = winner() == to_move;
        unset(i);
        if (won) {
          return true;
        }
      }
    }
    return false;
  }

  // The higher the better for the player to move.
  //
  // The return value is in the range [0, WIN].
  //
  // Stops early if no path can be better than the provided limit.
  ScoreType evaluate(ScoreType limit) {
    const PlayerType to_move = get_player_to_move();
    ScoreType score = 0;
    // Can win with the next move?
    // Only test if we can win if that would be better than the limit.
    if (limit < WIN && winnable()) {
      score = WIN;
    } else if (limit < decay(decay(WIN))) {
      // Only test if we can win after two plays if it can be even better.
      for (int i = 0; i < TILES; i++) {
        if (is_free(i)) {
          set(i, to_move);
          const ScoreType evaluation = decay(WIN - evaluate(max(limit, score)));
          score = max(score, evaluation);
          unset(i);
        }
      }
    }
    return score;
  }

  void for_all_possibilities(function<void(size_t)> op) {
    for (size_t i = 0; i < 9; i++) {
      if (is_free(i)) {
        op(i);
      }
    }
  }

  // Makes the best possible play for the player to move.
  size_t get_best_play() {
    // The bigger the better for the opponent.
    ScoreType best_so_far = WIN;
    size_t best_move = 0;
    for (int i = 0; i < TILES; i++) {
      if (is_free(i)) {
        set(i, get_player_to_move());
        ScoreType evaluation = evaluate(0);
        // Update when equal because the initial best move may not be valid.
        if (evaluation <= best_so_far) {
          best_so_far = evaluation;
          best_move = i;
        }
        unset(i);
      }
    }
    if (debugging) {
      cerr << "Evaluated to " << best_so_far << '\n';
    }
    return best_move;
  }

  PlayerType get(size_t i) const {
    return (board >> get_shift(i)) & 3;
  }

  bool is_free(size_t i) const {
    return get(i) == NONE;
  }

  bool is_full() const {
    for (size_t i = 0; i < 9; i++) {
      if (is_free(i)) {
        return false;
      }
    }
    return true;
  }

  void set(size_t i, PlayerType v) {
    // Zero the position.
    unset(i);
    // Set to the player code.
    BoardType value = v & 3;
    board |= value << get_shift(i);
  }

  void unset(size_t i) {
    board &= ~(3 << get_shift(i));
  }
};

ostream &operator<<(ostream &os, const TicTacToe &game) {
  for (int i = 0; i < 9; i++) {
    if (i % 3 == 0) {
      os << ' ';
    }
    os << game.get_symbol(game.get(i));
    if (i % 3 == 2) {
      os << '\n';
    }
  }
  os << '\n';
  return os;
}

istream &operator>>(istream &is, TicTacToe &game) {
  for (int i = 0; i < 9; i++) {
    char value;
    is >> value;
    game.set(i, game.get_player(value));
  }
  return is;
}

class Stopwatch {
 private:
  typedef chrono::high_resolution_clock Clock;
  typedef chrono::nanoseconds Duration;
  typedef Clock::time_point TimePoint;

  const string identifier = "";
  bool paused = true;
  Duration counter = Duration(0);
  TimePoint beginning;

  static string duration_to_string(Duration duration) {
    const vector<string> units = {"ns", "μs", "ms", "s"};
    int64_t value = duration.count();
    auto it = units.begin();
    while ((it + 1 != units.end()) && value >= 1000) {
      it++;
      value /= 1000;
    }
    stringstream ss;
    ss << value << ' ' << *it;
    return ss.str();
  }

 public:
  Stopwatch() : Stopwatch("") {
  }

  Stopwatch(string id) : identifier(id) {
  }

  void start() {
    if (paused) {
      beginning = Clock::now();
      paused = false;
    }
  }

  void pause() {
    if (!paused) {
      counter += Clock::now() - beginning;
      paused = true;
    }
  }

  void print() {
    Duration count = counter;
    if (!paused) {
      count += (Clock::now() - beginning);
    }
    if (identifier != "") {
      cout << identifier << " took ";
    } else {
      cout << "Took ";
    }
    cout << duration_to_string(count) << '.' << '\n';
  }
};

int main(int argc, char **argv) {
  ios_base::sync_with_stdio(false);

  string argument = "play";
  if (argc > 1) {
    argument = string(argv[1], argv[1] + strlen(argv[1]));
  }

  map<string, function<void(void)>> actions;
  map<string, string> helpers;

  actions["help"] = [&actions, &helpers] {
    const size_t width = 20;
    for (auto p : actions) {
      const string action = p.first;
      cout << action;
      for (size_t w = p.first.size(); w < width; w++) {
        cout << " ";
      }
      cout << helpers[action] << '\n';
    }
  };
  helpers["help"] = "Prints infromation about each possible action.";

  actions["play"] = [&actions, &helpers] {
    TicTacToe game;
    bool x = true;
    cout << game;
    Stopwatch human("You");
    Stopwatch computer("The computer");
    while (game.winner() == game.get_player(' ') && !game.is_full()) {
      if (x) {
        human.start();
        bool done = false;
        size_t r;
        size_t c;
        while (!done) {
          cout << "Move: ";
          cin >> r >> c;
          r--;
          c--;
          done = game.is_free(3 * r + c);
        }
        game.set(3 * r + c, game.get_player('X'));
        human.pause();
        cout << "After you:";
      } else {
        computer.start();
        size_t best = game.get_best_play();
        size_t r = best / 3;
        size_t c = best % 3;
        game.set(3 * r + c, game.get_player('O'));
        computer.pause();
        cout << "After the computer:";
      }
      cout << "\n\n";
      cout << game;
      x = !x;
    }
    human.print();
    computer.print();
  };
  helpers["play"] = "Starts a game against the AI.";

  actions["watch"] = [&actions, &helpers] {
    TicTacToe game;
    bool x = true;
    cout << game;
    Stopwatch stopwatch;
    stopwatch.start();
    while (!game.is_full()) {
      size_t best = game.get_best_play();
      size_t r = best / 3;
      size_t c = best % 3;
      char s = x ? 'X' : 'O';
      x = !x;
      game.set(3 * r + c, game.get_player(s));
      cout << game;
    }
    stopwatch.print();
  };
  helpers["watch"] = "Outputs a game of the AI against the AI.";

  if (actions.count(argument)) {
    actions[argument]();
  } else {
    actions["help"]();
  }

  return 0;
}
