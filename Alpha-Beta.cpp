#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <queue>

struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(int x, int y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;
class Othelloboard
{
public:
    enum SPOT_STATE { EMPTY = 0, BLACK = 1, WHITE = 2 };
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    std::vector<Point> next_valid_spots;
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Othelloboard> next_step_Othelloboard;
    int alpha = 0; //the maximum score that the player is assured of in the current search process
    int beta = 1000; //the minimum score that the opponent is assured of in the current search process
    int player;

private:
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (get_disc(p) == player) {
                    for (Point s: discs) {
                        set_disc(s, player);
                    }
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }

public:
    Othelloboard(std::array<std::array<int, SIZE>, SIZE> b, int p) :board(b), player(p){
        //next_valid_spots = get_valid_spots();
    };
    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
//for (auto s: valid_spots) std::cout << "get valid spots: (" << s.x << "," << s.y  << ")" << std::endl;
        return valid_spots;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (get_disc(p) != (3^player))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (get_disc(p) == player)
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void add_next_step_Othelloboard() {
        next_valid_spots = get_valid_spots();
std::cout << "start add next step othelloboard" << std::endl;
        for (Point p: next_valid_spots) {
//std::cout << "next_valid_spots: (" << p.x << "," << p.y << ")" << std::endl;
            Othelloboard new_othelloboard(board, player);
//std::cout << "player: " << new_othelloboard.player << std::endl;
            new_othelloboard.flip_discs(p);
//for (int i = 0; i < 8; i++) {
//for (int j = 0; j < 8; j++) std::cout << new_othelloboard.board[i][j] << " ";
//std::cout << std::endl;
//}
            new_othelloboard.player = (new_othelloboard.player^3);
            next_step_Othelloboard.push_back(new_othelloboard);
        }
    }
    int count_flip_discs(Point center) {
        int count = 0;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            int count_dir = 0;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (get_disc(p) == player) {
                    count += count_dir;
                    break;
                }
                count_dir++;
                p = p + dir;
            }
        }
        return count;
    };
};


void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back({x, y});
    }
}

void write_valid_spot(std::ofstream& fout) {
    Othelloboard first_step(board, player);
    first_step.add_next_step_Othelloboard();
    for (Othelloboard &o: first_step.next_step_Othelloboard) {
        o.add_next_step_Othelloboard();
        for (Othelloboard &t: o.next_step_Othelloboard) {
            t.next_valid_spots = t.get_valid_spots();
            for (Point &p: t.next_valid_spots) {
                int a = t.count_flip_discs(p);
//std::cout << "flip discs: " << a << std::endl;
                if (a > t.alpha) t.alpha = a;
            }
std::cout << "t.alpha: " << t.alpha << std::endl;
            if (t.alpha < o.beta) o.beta = t.alpha;
        }
std::cout << "o.beta: " << o.beta << std::endl;
    }
    int index = 0;
    int beta_max = 0;
    int s = next_valid_spots.size();
std::cout << "size: " << s << std::endl;
    for (int i = 0; i < s; i++) {
        if (first_step.next_step_Othelloboard.at(i).beta > beta_max) {
            beta_max = first_step.next_step_Othelloboard.at(i).beta;
            index = i;
        }
    }
std::cout << "index: " << index << std::endl;
    Point p = next_valid_spots.at(index);
    // Remember to flush the output to ensure the last action is written to file.
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}

int main(int, char** argv) {
    std::cout << "start" << std::endl;
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}

