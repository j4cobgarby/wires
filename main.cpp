#include <SFML/Graphics.hpp>
#include <iostream>
#include <array>
#include <math.h>

#define WIN_WIDTH  1024
#define WIN_HEIGHT 1024
#define CELLS_X 100
#define CELLS_Y 100
#define DELTA_TICKTIME 0.02

#define EMPTY         0
#define CONDUCTOR     1
#define ELECTRON_HEAD 2
#define ELECTRON_TAIL 3

class Grid {
protected:
    int cells_x, cells_y;
    float cell_size;
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<sf::RectangleShape>> rects;

    sf::Color get_color(int val);
public:
    Grid(int cells_x, int cells_y);

    void update_rects();
    void update_rect(int x, int y);
    void set_cell(int x, int y, int val);
    int get_cell(int x, int y);
    float get_cell_size() {return cell_size;}

    void tick();
    void draw(sf::RenderWindow& win);
};

std::string num_to_brush(int num) {
    switch (num) {
        case EMPTY: return "Erase";
        case CONDUCTOR: return "Conductor";
        case ELECTRON_HEAD: return "e Head";
        case ELECTRON_TAIL: return "e Tail";
        default: return "Invalid";
    }
}

int main() {
    sf::RenderWindow win(sf::VideoMode(WIN_WIDTH, WIN_HEIGHT), "Wireworld");
    sf::Font mono;
    sf::Text infotxt;
    sf::Clock tick_clock;
    
    Grid grid(CELLS_X, CELLS_Y);

    int brush = 0;
    int running = false;
    float ticktime = 0.06;
    
    if (!mono.loadFromFile("DejaVuSansMono.ttf")) {
        std::cout << "Font not found" << std::endl;
        return EXIT_FAILURE;
    }

    infotxt.setFont(mono);
    infotxt.setCharacterSize(17);
    infotxt.setFillColor(sf::Color(0x888888ff));
    infotxt.setOutlineColor(sf::Color::Black);
    infotxt.setOutlineThickness(1);

    while (win.isOpen()) {
        sf::Event ev;
        while (win.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) win.close();
            if (ev.type == sf::Event::MouseWheelScrolled) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
                    if (ev.mouseWheelScroll.delta > 0) ticktime += DELTA_TICKTIME;
                    else ticktime -= DELTA_TICKTIME;
                } else {
                    if (ev.mouseWheelScroll.delta > 0) brush+1 > 3 ? 3 : brush++;
                    else brush-1 < 0 ? 0 : brush--;
                }
            }
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Space) running = ! running;
                if (ev.key.code == sf::Keyboard::C) {
                    for (int y = 0; y < CELLS_Y; y++) {
                        for (int x = 0; x < CELLS_X; x++) {
                            grid.set_cell(x, y, EMPTY);
                        }
                    }
                }
                if (ev.key.code == sf::Keyboard::R) {
                    for (int y = 0; y < CELLS_Y; y++) {
                        for (int x = 0; x < CELLS_X; x++) {
                            if (grid.get_cell(x, y) >= 2) { // Head or tail
                                grid.set_cell(x, y, CONDUCTOR);
                            }
                        }
                    }
                }
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
            sf::Vector2i m_cell = sf::Mouse::getPosition(win);
            try {
                grid.set_cell(
                    (int)floor((float)m_cell.x/grid.get_cell_size()), 
                    (int)floor((float)m_cell.y/grid.get_cell_size()), 
                    sf::Mouse::isButtonPressed(sf::Mouse::Button::Right) ? EMPTY : brush);
            } catch (...) {} 
        }

        infotxt.setString((std::string)"~~ INFORMATION ~~" +
                        "\nbrush .......... " + num_to_brush(brush) + 
                        "\nrunning ........ " + std::to_string(running) +
                        "\nticktime ....... " + std::to_string(ticktime) +
                        "\n\n~~ CONTROLS ~~" +
                        "\nC .............. Clear screen" +
                        "\nR .............. Remove all electrons" +
                        "\nT+<scroll> ..... Change time delay" +
                        "\n<scroll> ....... Cycle brushes" +
                        "\n<left click> ... Use brush" +
                        "\n<right click> .. Erase");

        if (running && tick_clock.getElapsedTime().asSeconds() > ticktime) {
            tick_clock.restart();
            grid.tick();
        }

        win.clear();
        grid.draw(win);
        win.draw(infotxt);
        win.display();
    }
}

void Grid::tick() {
    std::vector<std::vector<int>> temp(grid);

    for (int y = 0; y < CELLS_Y; y++) {
        for (int x = 0; x < CELLS_X; x++) {
            if (get_cell(x, y) == ELECTRON_HEAD) temp.at(y).at(x) = ELECTRON_TAIL;
            if (get_cell(x, y) == ELECTRON_TAIL) temp.at(y).at(x) = CONDUCTOR;
            if (get_cell(x, y) == CONDUCTOR) {
                int neighbours = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        try {
                            if (get_cell(x+dx, y+dy) == ELECTRON_HEAD) neighbours++;
                        } catch (...) {}
                    }
                }
                if (neighbours == 1 || neighbours == 2) temp.at(y).at(x) = ELECTRON_HEAD;
            }
        }
    }

    grid.swap(temp);
    update_rects();
}

Grid::Grid(int cells_x, int cells_y) : cells_x(cells_x), cells_y(cells_y) {
    cell_size = (float)WIN_WIDTH/(float)CELLS_X;

    for (int y = 0; y < CELLS_Y; y++) {
        grid.push_back(std::vector<int>());
        rects.push_back(std::vector<sf::RectangleShape>());
        for (int x = 0; x < CELLS_X; x++) {
            // Append integer
            grid.at(y).push_back(EMPTY);

            // Setup rectangle
            rects.at(y).push_back(sf::RectangleShape(sf::Vector2f(cell_size, cell_size)));
            rects.at(y).at(x).setPosition(x*cell_size, y*cell_size);
        }
    }

    update_rects();
}

void Grid::update_rects() {
    for (int y = 0; y < CELLS_Y; y++) {
        for (int x = 0; x < CELLS_X; x++) {
            update_rect(x, y);
        }
    }
}

void Grid::update_rect(int x, int y) {
    rects.at(y).at(x).setFillColor(get_color(grid.at(y).at(x))); // Get colour from cell at x, y
}

sf::Color Grid::get_color(int val) {
    switch (val) {
        case EMPTY:         return sf::Color::Black;
        case CONDUCTOR:     return sf::Color::Yellow;
        case ELECTRON_HEAD: return sf::Color::Blue;
        case ELECTRON_TAIL: return sf::Color::Red;
        default:            return sf::Color::Black;
    };
}

void Grid::draw(sf::RenderWindow& win) {
    for (int y = 0; y < CELLS_Y; y++) {
        for (int x = 0; x < CELLS_X; x++) {
            win.draw(rects.at(y).at(x));
        }
    }
}

void Grid::set_cell(int x, int y, int val) {
    grid.at(y).at(x) = val;
    update_rect(x, y);
}

int Grid::get_cell(int x, int y) {
    return grid[y][x];
}