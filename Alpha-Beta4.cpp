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
    int alpha = -1000; //the maximum score that the player is assured of in the current search process
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
//std::cout << "start add next step othelloboard" << std::endl;
        for (Point p: next_valid_spots) {
//std::cout << "next_valid_spots: (" << p.x << "," << p.y << ")" << std::endl;
            Othelloboard new_othelloboard(board, player);
//std::cout << "player: " << new_othelloboard.player << std::endl;
            new_othelloboard.flip_discs(p);
            new_othelloboard.player = (new_othelloboard.player^3);
            next_step_Othelloboard.push_back(new_othelloboard);
        }
        if (next_valid_spots.size() == 0) next_step_Othelloboard.push_back(Othelloboard(board, (player^3)));
    }
    int count_player_discs(Point center) {
        int count = 0;
        Othelloboard temp = *this;
        temp.flip_discs(center);
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (temp.board[i][j] == player) count++;
                if (temp.board[i][j] == (3^player)) count--;
                //if (temp.board[i][j] == player && (i == 0 || i == 7 || j == 0 || j == 7))
                //    count += 1;
                //if (temp.board[i][j] == player && (i == 1 || i == 6 || j == 1 || j == 6))
                //    count -= 1;
            }
        }
        if (temp.board[0][0] == player) count += 10;
        if (temp.board[0][7] == player) count += 10;
        if (temp.board[7][0] == player) count += 10;
        if (temp.board[7][7] == player) count += 10;
        if (temp.board[0][0] == (3^player)) count -= 10;
        if (temp.board[0][7] == (3^player)) count -= 10;
        if (temp.board[7][0] == (3^player)) count -= 10;
        if (temp.board[7][7] == (3^player)) count -= 10;/*
        if (temp.board[1][1] == player && temp.board[0][0] == EMPTY) count -= 5;
        if (temp.board[1][6] == player && temp.board[0][7] == EMPTY) count -= 5;
        if (temp.board[6][1] == player && temp.board[7][0] == EMPTY) count -= 5;
        if (temp.board[6][6] == player && temp.board[7][7] == EMPTY) count -= 5;
        if (temp.board[1][1] == (3^player) && temp.board[0][0] == EMPTY) count += 5;
        if (temp.board[1][6] == (3^player) && temp.board[0][7] == EMPTY) count += 5;
        if (temp.board[6][1] == (3^player) && temp.board[7][0] == EMPTY) count += 5;
        if (temp.board[6][6] == (3^player) && temp.board[7][7] == EMPTY) count += 5;

        if (temp.board[0][1] == player && temp.board[0][0] == (3^player)) count -= 5;
        if (temp.board[1][0] == player && temp.board[0][7] == (3^player)) count -= 5;
        if (temp.board[0][6] == player && temp.board[7][0] == (3^player)) count -= 5;
        if (temp.board[6][0] == player && temp.board[7][7] == (3^player)) count -= 5;
        if (temp.board[1][7] == player && temp.board[0][0] == (3^player)) count -= 5;
        if (temp.board[7][1] == player && temp.board[0][7] == (3^player)) count -= 5;
        if (temp.board[6][7] == player && temp.board[7][0] == (3^player)) count -= 5;
        if (temp.board[7][6] == player && temp.board[7][7] == (3^player)) count -= 5;
*/

        
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
void write_valid_spot_notime(std::ofstream& fout) {
    Point notime = next_valid_spots.at(0);
    fout << notime.x << " " << notime.y << std::endl;
    fout.flush();
    std::cout << "NOTIME.........................................................................Notime" << std::endl;
}

void write_valid_spot_limit_time(std::ofstream& fout) {
    Othelloboard first_step(board, player);
    first_step.add_next_step_Othelloboard();
    for (Othelloboard &o: first_step.next_step_Othelloboard) {
        o.add_next_step_Othelloboard();
        for (Othelloboard &p2: o.next_step_Othelloboard) {
            p2.add_next_step_Othelloboard();
            for (Othelloboard &o2: p2.next_step_Othelloboard) {
                o2.add_next_step_Othelloboard();
                for (Othelloboard &p3 : o2.next_step_Othelloboard) {
                    p3.add_next_step_Othelloboard();
                    for (Point P : p3.next_valid_spots) {
                        int a = p3.count_player_discs(P);
                        if (a > o2.beta) { p3.alpha = a; break; }
                        if (a > p3.alpha) p3.alpha = a;
                    }
                    if (p3.alpha < o2.beta) o2.beta = p3.alpha;
                }
                if (o2.beta > o.beta) { p2.alpha = o2.beta; break; }
                if (o2.beta > p2.alpha) p2.alpha = o2.beta;
            }
            if (p2.alpha < o.beta) o.beta = p2.alpha;
        }
        if (o.beta > first_step.alpha) first_step.alpha = o.beta;
    }
    int index = 0;
    int beta_max = -1;
    int s = next_valid_spots.size();
//std::cout << "size: " << s << std::endl;
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
    std::cout << "Limit_time....................................................................Limit_time" << std::endl;
}

void write_valid_spot(std::ofstream& fout) {
    Othelloboard first_step(board, player);
    first_step.add_next_step_Othelloboard();
    for (Othelloboard &o: first_step.next_step_Othelloboard) {
        o.add_next_step_Othelloboard();
        for (Othelloboard &p2: o.next_step_Othelloboard) {
            p2.add_next_step_Othelloboard();
            for (Othelloboard &o2: p2.next_step_Othelloboard) {
                o2.add_next_step_Othelloboard();
                for (Othelloboard &p3 : o2.next_step_Othelloboard) {
                    p3.add_next_step_Othelloboard();
                    for (Othelloboard &o3 : p3.next_step_Othelloboard) {
                        o3.add_next_step_Othelloboard();
                        for (Othelloboard &p4 : o3.next_step_Othelloboard) {
                            p4.next_valid_spots = p4.get_valid_spots();
                            for (Point P : p4.next_valid_spots) {
                                int a = p4.count_player_discs(P);
                                if (a > o3.beta) { p4.alpha = a; break; }
                                if (a > p4.alpha) p4.alpha = a;
                            }
                            if (p4.alpha < o3.beta) o3.beta = p4.alpha;
                        }
                        if (o3.beta > o2.beta) { p3.alpha = o3.beta; break; }
                        if (o3.beta > p3.alpha) p3.alpha = o3.beta;
                    }
                    if (p3.alpha < o2.beta) o2.beta = p3.alpha;
                }
                if (o2.beta > o.beta) { p2.alpha = o2.beta; break; }
                if (o2.beta > p2.alpha) p2.alpha = o2.beta;
            }
            if (p2.alpha < o.beta) o.beta = p2.alpha;
        }
        if (o.beta > first_step.alpha) first_step.alpha = o.beta;
    }
    int index = 0;
    int beta_max = -1;
    int s = next_valid_spots.size();
//std::cout << "size: " << s << std::endl;
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
    std::cout << "Enoutgh time-------------------------------------------------------------------------------Enough time" << std::endl;
}
void write_valid_spot_deep(std::ofstream& fout) {
    Othelloboard first_step(board, player);
    first_step.add_next_step_Othelloboard();
    for (Othelloboard &o: first_step.next_step_Othelloboard) {
        o.add_next_step_Othelloboard();
        for (Othelloboard &p2: o.next_step_Othelloboard) {
            p2.add_next_step_Othelloboard();
            for (Othelloboard &o2: p2.next_step_Othelloboard) {
                o2.add_next_step_Othelloboard();
                for (Othelloboard &p3 : o2.next_step_Othelloboard) {
                    p3.add_next_step_Othelloboard();
                    for (Othelloboard &o3 : p3.next_step_Othelloboard) {
                        o3.add_next_step_Othelloboard();
                        for (Othelloboard &p4 : o3.next_step_Othelloboard) {
                            p4.add_next_step_Othelloboard(); 
                            for (Othelloboard &o4 : p4.next_step_Othelloboard) {
                                o4.add_next_step_Othelloboard();
                                for (Othelloboard &p5 : o4.next_step_Othelloboard) {
                                    p5.next_valid_spots = p5.get_valid_spots();
                                    for (Point P : p5.next_valid_spots) {
                                        int a = p5.count_player_discs(P);
                                        if (a > o4.beta) { p5.alpha = a; break; }
                                        if (a > p5.alpha) p5.alpha = a;
                                    }
                                    if (p5.alpha < o4.beta) o4.beta = p5.alpha;
                                }
                                if (o4.beta > o3.beta) { p4.alpha = o4.beta; break; }
                                if (o4.beta > p4.alpha) p4.alpha = o4.beta;
                            }
                            if (p4.alpha < o3.beta) o3.beta = p4.alpha;
                        }
                        if (o3.beta > o2.beta) { p3.alpha = o3.beta; break; }
                        if (o3.beta > p3.alpha) p3.alpha = o3.beta;
                    }
                    if (p3.alpha < o2.beta) o2.beta = p3.alpha;
                }
                if (o2.beta > o.beta) { p2.alpha = o2.beta; break; }
                if (o2.beta > p2.alpha) p2.alpha = o2.beta;
            }
            if (p2.alpha < o.beta) o.beta = p2.alpha;
        }
        if (o.beta > first_step.alpha) first_step.alpha = o.beta;
    }
    int index = 0;
    int beta_max = -100;
    int s = next_valid_spots.size();
//std::cout << "size: " << s << std::endl;
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
    std::cout << "DEEP------------------------------------------------------------------------------DEEP" << std::endl;
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot_notime(fout);
    write_valid_spot_limit_time(fout);
    write_valid_spot(fout);
    write_valid_spot_deep(fout);
    fin.close();
    fout.close();
    return 0;
}

