#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <limits>
#include <string>
#include <set>
#include <algorithm>
using namespace std;

// Структура для представления узла графа
struct Node {
    double lon, lat; // Долгота и широта узла
    vector<pair<Node*, double>> edges; // Вектор соседей (указатель на узел и вес ребра)
};

// Структура для представления графа
struct Graph {
    vector<Node*> nodes; // Вектор всех узлов графа
    unordered_map<string, Node*> node_map; // Хранение узлов по координатам (ключ - строка, содержащая координаты)

    // Метод для поиска ближайшего узла к заданным координатам
    Node* find_closest_node(double lat, double lon) {
        double min_distance = numeric_limits<double>::max(); // Минимальная дистанция, инициализируется как бесконечность
        Node* node_founded = nullptr; // Переменная для хранения найденного узла

        // Перебор всех узлов графа
        for (auto node : nodes) {
            double distance = sqrt(pow(node->lat - lat, 2) + pow(node->lon - lon, 2)); // Евклидово расстояние
            if (distance < min_distance) {
                node_founded = node;
                min_distance = distance;
            }
        }

        return node_founded; // Возвращаем ближайший узел
    }

    // Метод для разделения строки на части по заданному разделителю
    vector<string> split(string s, char del) {
        stringstream ss(s);
        string word;
        vector<string> tokens;

        while (getline(ss, word, del)) {
            tokens.push_back(word);
        }

        return tokens;
    }

    // Метод для получения узла или создания нового, если его еще нет
    Node* get_or_create_node(double lat, double lon) {
        string key = to_string(lat) + "," + to_string(lon); // Формируем ключ на основе координат
        if (node_map.find(key) == node_map.end()) { // Если узла с таким ключом нет
            Node* new_node = new Node{lon, lat, {}}; // Создаем новый узел
            node_map[key] = new_node;
            nodes.push_back(new_node);
        }
        return node_map[key]; // Возвращаем найденный или созданный узел
    }

    // Метод для обработки строки и добавления узлов и рёбер в граф
    void processing_line(string line) {
        vector<string> parts = split(line, ':'); // Разделяем строку на координаты узла и список рёбер
        vector<string> coords = split(parts[0], ',');

        double lat = stod(coords[0]); // Широта
        double lon = stod(coords[1]); // Долгота

        Node* parent_node = get_or_create_node(lat, lon); // Получаем или создаём узел

        vector<string> edges = split(parts[1], ';'); // Разделяем список рёбер
        for (string edge : edges) {
            vector<string> edge_parts = split(edge, ',');

            double subsidiary_lat = stod(edge_parts[0]);
            double subsidiary_lon = stod(edge_parts[1]);
            double weight = stod(edge_parts[2]);

            Node* subsidiary_node = get_or_create_node(subsidiary_lat, subsidiary_lon);

            // Добавляем ребро в оба направления (граф неориентированный)
            parent_node->edges.push_back(make_pair(subsidiary_node, weight));
            subsidiary_node->edges.push_back(make_pair(parent_node, weight));
        }
    }

    // Метод для чтения графа из файла
    void read_graph(string filename) {
        ifstream file(filename);

        string line;
        while (getline(file, line)) {
            processing_line(line); // Обрабатываем каждую строку
        }
    }

    // Поиск пути с помощью DFS
    vector<Node*> dfs(double start_lat, double start_lon, double target_lat, double target_lon) {
        Node* start_node = find_closest_node(start_lat, start_lon); // Находим ближайший стартовый узел
        Node* target_node = find_closest_node(target_lat, target_lon); // Находим ближайший целевой узел

        vector<Node*> best_path, path;
        set<Node*> visited; // Множество посещенных узлов

        path.push_back(start_node); // Добавляем начальный узел в путь
        visited.insert(start_node);

        while (!path.empty()) {
            Node* current = path.back(); // Текущий узел

            if (current == target_node) { // Если достигли целевого узла
                if (best_path.empty() || path.size() < best_path.size()) { // Сохраняем путь, если он лучше
                    best_path = path;
                    if (path.size() == 1) { // Если путь состоит из одного узла
                        return best_path;
                    }
                }
                path.pop_back(); // Возвращаемся назад
                continue;
            }

            bool flag = true;
            for (auto& edge : current->edges) {
                Node* neighbor = edge.first;
                if (visited.find(neighbor) == visited.end()) { // Если сосед не посещен
                    path.push_back(neighbor);
                    visited.insert(neighbor);
                    flag = false;
                    break;
                }
            }

            if (flag) {
                path.pop_back(); // Если больше нет соседей, возвращаемся назад
            }
        }

        return best_path; // Возвращаем лучший путь
    }

    // BFS поиск пути
    vector<Node*> bfs(double start_lat, double start_lon, double target_lat, double target_lon) {
        Node* start_node = find_closest_node(start_lat, start_lon);
        Node* target_node = find_closest_node(target_lat, target_lon);

        vector<Node*> stack; // Очередь для BFS
        set<Node*> visited; // Множество посещенных узлов
        unordered_map<Node*, Node*> parent; // Карта для восстановления пути

        stack.push_back(start_node);
        visited.insert(start_node);

        while (!stack.empty()) {
            Node* current = stack[0];
            stack.erase(stack.begin());

            if (current == target_node) { // Если достигли цели
                vector<Node*> path;
                while (current) {
                    path.push_back(current);
                    current = parent[current];
                }

                reverse(path.begin(), path.end());
                return path;
            }

            for (auto& edge : current->edges) {
                Node* neighbor = edge.first;
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    parent[neighbor] = current;
                    stack.push_back(neighbor);
                }
            }
        }

        return {}; // Путь не найден
    }

    // Алгоритм Дейкстры
    vector<Node*> dijkstra(double start_lat, double start_lon, double target_lat, double target_lon) {
        Node* start_node = find_closest_node(start_lat, start_lon);
        Node* target_node = find_closest_node(target_lat, target_lon);

        unordered_map<Node*, double> distances; // Расстояния до узлов
        unordered_map<Node*, Node*> parents; // Карта для восстановления пути
        set<pair<double, Node*>> stack; // Очередь с приоритетом

        for (Node* node : nodes) {
            distances[node] = numeric_limits<double>::max();
        }
        distances[start_node] = 0.0;
        stack.insert({0.0, start_node});

        while (!stack.empty()) {
            Node* current = stack.begin()->second;
            stack.erase(stack.begin());

            if (current == target_node) { // Если достигли цели
                break;
            }

            for (auto& edge : current->edges) {
                Node* neighbor = edge.first;
                double weight = edge.second;

                double new_distance = distances[current] + weight;
                if (new_distance < distances[neighbor]) { // Если нашли более короткий путь
                    stack.erase({distances[neighbor], neighbor});
                    distances[neighbor] = new_distance;
                    parents[neighbor] = current;
                    stack.insert({new_distance, neighbor});
                }
            }
        }

        vector<Node*> path;
        Node* current = target_node;
        while (current) {
            path.push_back(current);
            current = parents[current];
        }

        reverse(path.begin(), path.end());
        return path;
    }

    // Алгоритм A*
    double metric(Node* a, Node* b) {
        double dx = a->lon - b->lon;
        double dy = a->lat - b->lat;
        return sqrt(dx * dx + dy * dy); // Эвристика: Евклидово расстояние
    }

    vector<Node*> Astar(double start_lat, double start_lon, double target_lat, double target_lon) {
        Node* start_node = find_closest_node(start_lat, start_lon);
        Node* target_node = find_closest_node(target_lat, target_lon);

        unordered_map<Node*, double> f; // Стоимость пути
        unordered_map<Node*, double> h; // Эвристика
        unordered_map<Node*, Node*> parents; // Карта для восстановления пути
        set<pair<double, Node*>> stack; // Очередь с приоритетом

        for (Node* node : nodes) {
            f[node] = numeric_limits<double>::infinity();
            h[node] = numeric_limits<double>::infinity();
        }
        f[start_node] = 0.0;
        h[start_node] = metric(start_node, target_node);
        stack.insert({h[start_node], start_node});

        while (!stack.empty()) {
            Node* current = stack.begin()->second;
            stack.erase(stack.begin());

            if (current == target_node) { // Если достигли цели
                vector<Node*> path;
                while (current) {
                    path.push_back(current);
                    current = parents[current];
                }
                reverse(path.begin(), path.end());
                return path;
            }

            for (pair<Node*, double> edge : current->edges) {
                Node* neighbor = edge.first;
                double weight = edge.second;

                double tentative_f = f[current] + weight;
                if (tentative_f < f[neighbor]) {
                    parents[neighbor] = current;
                    f[neighbor] = tentative_f;
                    h[neighbor] = f[neighbor] + metric(neighbor, target_node);

                    stack.insert({h[neighbor], neighbor});
                }
            }
        }

        return {}; // Путь не найден
    }

};


int main() {
    Graph graph; // Создаём граф
    string filename = "spb_graph-2.txt"; // Файл с данными
    vector<Node*> path;

    // Считываем граф из файла
    graph.read_graph(filename);

    auto start = chrono::high_resolution_clock::now();
    path = graph.dfs(30.3585261, 59.8864419, 30.3027079, 59.9570161);
    cout << "Раземер пути "<< path.size() << endl;
    auto end = std::chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << elapsed.count() << " DFS секунд"<< endl;;

    if (!path.empty()) {
        cout << "Кратчайший путь:" << endl;
        for (const auto& node : path) {
            cout << "(" << node->lat << ", " << node->lon << ") => ";
        }
        cout << "конец" << endl;
    } else {
        cout << "Путь между указанными узлами не найден." << endl;
    }

    start = chrono::high_resolution_clock::now();
    path = graph.bfs(30.3585261, 59.8864419, 30.3027079, 59.9570161);
    cout << "Раземер пути "<< path.size() << endl;
    end = chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << elapsed.count() << " BFS секунд" << endl;

    if (!path.empty()) {
        cout << "Кратчайший путь:" << endl;
        for (const auto& node : path) {
            cout << "(" << node->lat << ", " << node->lon << ") => ";
        }
        cout << "конец" << endl;
    } else {
        cout << "Путь между указанными узлами не найден." << endl;
    }

    start = chrono::high_resolution_clock::now();
    path = graph.dijkstra(30.3585261, 59.8864419, 30.3027079, 59.9570161);
    cout << "Раземер пути "<< path.size() << endl;
    end = chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << elapsed.count() << " Дейкстра секунд" << endl;

    if (!path.empty()) {
        cout << "Кратчайший путь (по весам):" << endl;
        for (const auto& node : path) {
            cout << "(" << node->lat << ", " << node->lon << ") => ";
        }
        cout << "конец" << endl;
    } else {
        cout << "Путь между указанными узлами не найден." << endl;
    }

    start = chrono::high_resolution_clock::now();
    path = graph.Astar(30.3585261, 59.8864419, 30.3027079, 59.9570161);
    cout << "Раземер пути "<< path.size() << endl;
    end = chrono::high_resolution_clock::now();
    elapsed = end - start;
    cout << elapsed.count() << " А* секунд"<< endl;;

    if (!path.empty()) {
        cout << "Кратчайший путь (по весам):" << endl;
        for (const auto& node : path) {
            cout << "(" << node->lat << ", " << node->lon << ") => ";
        }
        cout << "конец" << endl;
    } else {
        cout << "Путь между указанными узлами не найден." << endl;
    }

    // Печатаем граф
//    for (const auto& node : graph.nodes) {
//        cout << "Узел (" << node->lat << ", " << node->lon << "): ";
//        for (const auto& edge : node->edges) {
//            cout << " -> (" << edge.first->lat << ", " << edge.first->lon << ") [Вес: " << edge.second << "]";
//        }
//        cout << endl;
//    }

    return 0;
}