#include <QApplication>
#include <QPushButton>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QDebug>
#include <QPainter>
#include <QLabel>
#include <QtMath>
#include <vector>
#include <climits>
#include <algorithm>
#include <QMessageBox>

using namespace std;

bool isInVector(vector<int> v, int e)
{
    return find(v.begin(), v.end(), e) != v.end();
}

class Graph
{
public:
    Graph(vector<vector<int>> m, int s)
    {
        setM(m, s);
    }

    void setM(vector<vector<int>> m, int s)
    {
        matrix = m;
        size = s;
    }

    int findMinSum1(vector<int> strs, vector<int> stbs)
    {
        int sum = 0;
        int str_pos = 0;
        for (auto it = matrix.begin(); it!=matrix.end(); ++it)
        {
            if(!isInVector(strs, str_pos))
            {
                int stb_pos = 0;
                int m = INT_MAX;
                for (const auto &i:*it)
                {
                    if(i < m && !isInVector(stbs, stb_pos)) m = i;
                    ++stb_pos;
                };
                sum += m;
            }
            ++str_pos;
        }
        return sum;
    }

    int findMinSum2(vector<int> strs, vector<int> stbs)
    {
        int sum = 0;
        int stb_pos = 0;
        for (auto it = matrix[0].begin(); it!=matrix[0].end(); ++it)
        {
            if(!isInVector(stbs, stb_pos))
            {
                int str_pos = 0;
                int m = INT_MAX;
                for (auto i = matrix.begin(); i != matrix.end(); ++i)
                {
                    if((*i)[stb_pos] < m && !isInVector(strs, str_pos)) m = (*i)[stb_pos];
                    ++str_pos;
                };
                sum += m;
            }
            ++stb_pos;
        }
        return sum;
    }

    int getH(vector<int> strs, vector<int> stbs)
    {
        int H1 = findMinSum1(strs, stbs);
        int H2 = findMinSum2(strs, stbs);
        return max(H1, H2);
    }

    int getV(vector<int> last_nodes, int current_point)
    {
        int V1 = 0;
        for(int i = 0; i < this->size; ++i)
        {
            if(!isInVector(last_nodes, i) && i != current_point)
            {
                V1 += this->matrix[current_point][i];
                last_nodes.push_back(current_point);
                current_point = i;
            }
        }
        V1 += this->matrix[current_point][0];
        return V1;
    }

    vector<vector<int>> matrix;
    int size;
};

class BranchState {
public:
    int currentVertex;
    vector<int> path;
    int distance;
    int H;
    int V;
    int c;
    Graph* gr;

    BranchState(int vertex, const vector<int>& p, int dist, Graph* g)
        : currentVertex(vertex), path(p), distance(dist)
    {
        vector<int> strs;
        vector<int> stbs;

        for(int i = 0; i < this->path.size(); ++i)
        {
            strs.push_back(this->path[i]);
            if(i+1 < this->path.size())
                stbs.push_back(this->path[i+1]);
            else
                stbs.push_back(this->currentVertex);
        }

        this->H = this->distance + g->getH(strs, stbs);
        this->V = this->distance + g->getV(path, currentVertex);
        c = g->matrix.size();
        gr = g;
    }

    list<BranchState> getNextNodes()
    {
        list<BranchState> nodes;
        this->path.push_back(this->currentVertex);
        for(int i = 0; i < this->c; ++i)
        {
            if(!isInVector(this->path, i))
            {
                nodes.push_back(BranchState(i, this->path, this->distance, this->gr));
            }
        }
        return nodes;
    }
};

class Container
{
public:
    list<BranchState> nodes;
    int quantity;
    int r;
    Container()
    {
        this->quantity = 0;
        this->r = INT_MAX;
    }

    void add(BranchState node)
    {
        if(this->r >= node.V)
        {
            this->r = node.V;
        }
        nodes.push_back(node);
        ++this->quantity;
    }

    void addSome(list<BranchState> nodes)
    {
        for(auto &n : nodes)
        {
            this->add(n);
        }
    }

    void changeNodes(list<BranchState> nodes)
    {
        this->quantity = 0;
        this->nodes = {};
        this->addSome(nodes);
    }

    list<BranchState>::iterator del(list<BranchState>::iterator itr)
    {
        --this->quantity;
        return nodes.erase(itr);
    }
};

pair<int, vector<int>> solveTravelingSalesmanProblem(const vector<vector<int>>& graph) {
    int n = graph.size();
    Graph g(graph, n);

    BranchState first = BranchState(0, {}, 0, &g);
    Container cont = Container();
    cont.addSome(first.getNextNodes());

    int v = 0;

    while (true)
    {
        list<BranchState>::iterator itr = cont.nodes.begin();
        list<BranchState> new_nodes;
        int i = 0;
        while (itr != cont.nodes.end())
        {
            if (itr->H > cont.r && cont.nodes.size() != 1)
            {
                itr = cont.del(itr);
                ++v;
            }
            else
            {
                list<BranchState> nodes = itr->getNextNodes();
                new_nodes.insert(new_nodes.end(), nodes.begin(), nodes.end());
                ++itr;
            }
            ++i;
        }
        if(cont.quantity == 1 && new_nodes.size() == 0)
            break;
        cont.changeNodes(new_nodes);
    }

    BranchState final = cont.nodes.front();
    return make_pair(final.V, final.path);
}


class GraphWidget : public QWidget {
public:
    GraphWidget(QWidget* parent = nullptr) : QWidget(parent) {
        connect(addVertexButton, &QPushButton::clicked, this, &GraphWidget::addVertex);
        connect(removeVertexButton, &QPushButton::clicked, this, &GraphWidget::removeVertex);
        connect(addEdgeButton, &QPushButton::clicked, this, &GraphWidget::addEdge);
        connect(removeEdgeButton, &QPushButton::clicked, this, &GraphWidget::removeEdge);
        connect(calculateButton, &QPushButton::clicked, this, &GraphWidget::calculate);
    }

    void addVertex() {
        int n = graph.size();
        for (int i = 0; i < n; ++i) {
            graph[i].push_back(0);
        }
        vector<int> newRow(n + 1, 0);
        graph.push_back(newRow);
        update();
    }

    void removeVertex() {
        bool ok;
        int vertexIndex = QInputDialog::getInt(this, tr("Удалить вершину"), tr("Введите индекс вершины:"), 0, 0, INT_MAX, 1, &ok);
        if (ok && vertexIndex >= 0 && vertexIndex <= graph.size()) {
            graph.erase(graph.begin() + vertexIndex);
            for (auto& row : graph) {
                row.erase(row.begin() + vertexIndex);
            }
            update();
        }
    }

    void addEdge() {
        bool ok;
        int vertexIndex1 = QInputDialog::getInt(this, tr("Добавить ребро"), tr("Введите индекс первой вершины:"), 0, 0, INT_MAX, 1, &ok);
        if (ok && vertexIndex1 >= 0 && vertexIndex1 <= graph.size()) {
            int vertexIndex2 = QInputDialog::getInt(this, tr("Добавить ребро"), tr("Введите индекс второй вершины:"), 0, 0, INT_MAX, 1, &ok);
            if (ok && vertexIndex2 >= 0 && vertexIndex2 <= graph.size()) {
                int weight = QInputDialog::getInt(this, tr("Добавить ребро"), tr("Введите вес ребра:"), 0, 0, INT_MAX, 1, &ok);
                if (ok) {
                    graph[vertexIndex1][vertexIndex2] = weight;
                    graph[vertexIndex2][vertexIndex1] = weight;
                    update();
                }
            }
        }
    }

    void removeEdge() {
        bool ok;
        int vertexIndex1 = QInputDialog::getInt(this, tr("Удалить ребро"), tr("Введите индекс первой вершины:"), 0, 0, INT_MAX, 1, &ok);
        if (ok && vertexIndex1 >= 0 && vertexIndex1 < graph.size()) {
            int vertexIndex2 = QInputDialog::getInt(this, tr("Удалить ребро"), tr("Введите индекс второй вершины:"), 0, 0, INT_MAX, 1, &ok);
            if (ok && vertexIndex2 >= 0 && vertexIndex2 < graph.size()) {
                graph[vertexIndex1][vertexIndex2] = 0;
                graph[vertexIndex2][vertexIndex1] = 0;
                update();
            }
        }
    }

    void calculate() {
        pair<int, vector<int>> result = solveTravelingSalesmanProblem(graph);
        minDistance = result.first;
        minPath = result.second;

        update();
    }

    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.translate(250, 250);

        int radius = 20;
        int bR = 200;
        int tS = 20;

        double R = 2 * M_PI / int(graph.size());

        for (int i = 0; i < int(graph.size()); ++i) {
            for (int j = i + 1; j < int(graph.size()); ++j) {
                if (graph[i][j] != 0) {
                    double x1 = qCos(i * R) * bR;
                    double y1 = qSin(i * R) * bR;
                    double x2 = qCos(j * R) * bR;
                    double y2 = qSin(j * R) * bR;
                    double dx = x2 - x1;
                    double dy = y2 - y1;
                    double vlen = qSqrt(dx * dx + dy * dy);
                    double dx2 = dy / vlen * tS / 2;
                    double dy2 = -dx / vlen * tS / 2;
                    painter.drawLine(QPointF(x1 + dx / vlen * radius, y1 + dy / vlen * radius), QPointF(x2 - dx / vlen * radius, y2 - dy / vlen * radius));
                    painter.drawText(QRectF((x1 + x2) / 2 + dx2 - tS / 2 , (y1 + y2) / 2 + dy2 - tS / 2, tS, tS), Qt::AlignCenter, QString::number(graph[i][j]));
                }
            }
        }

        for (int i = 0; i < int(graph.size()); i++) {
            double x = qCos(i * R) * bR;
            double y = qSin(i * R) * bR;

            painter.drawEllipse(QPointF(x, y), radius, radius);
            painter.drawText(QRectF(x - radius, y - radius, radius * 2, radius * 2), Qt::AlignCenter, QString::number(i));
        }

        minDistance = 0;
        for (int i = 0; i < int(minPath.size()); i++)
        {
            if (i + 1 == int(minPath.size()))
            {
                minDistance += graph[minPath[i]][minPath[0]];
            }
            else
            {
                minDistance += graph[minPath[i]][minPath[i + 1]];
            }
        }

        painter.drawText(QRectF(-250, -250, 300, 30), Qt::AlignLeft, "Минимальная дистанция: " + QString::number(minDistance));
        QString pathText = "Кратчайший путь: ";
        for (int i = 0; i < int(minPath.size()); ++i) {
            pathText += QString::number(minPath[i]);
            if (i != int(minPath.size()) - 1) {
                pathText += " -> ";
            }
        }

        if (int(minPath.size()) > 1)
        {
            pathText += " -> 0";
        }

        painter.drawText(QRectF(-250, -220, 300, 30), Qt::AlignLeft, pathText);

        painter.setPen(Qt::red);
        for (int i = 0; i < int(minPath.size()) - 1; ++i) {
            int vertex1 = minPath[i];
            int vertex2 = minPath[i + 1];

            double x1 = qCos(vertex1 * R) * bR;
            double y1 = qSin(vertex1 * R) * bR;
            double x2 = qCos(vertex2 * R) * bR;
            double y2 = qSin(vertex2 * R) * bR;

            double dx = x2 - x1;
            double dy = y2 - y1;
            double vlen = qSqrt(dx * dx + dy * dy);
            painter.drawLine(QPointF(x1 + dx / vlen * radius, y1 + dy / vlen * radius), QPointF(x2 - dx / vlen * radius, y2 - dy / vlen * radius));
        }

    }
    QPushButton* addVertexButton = new QPushButton("Добавить вершину");
    QPushButton* removeVertexButton = new QPushButton("Удалить вершину");
    QPushButton* addEdgeButton = new QPushButton("Добавить ребро");
    QPushButton* removeEdgeButton = new QPushButton("Удалить ребро");
    QPushButton* calculateButton = new QPushButton("Просчитать");
private:
    vector<vector<int>> graph; // Матрица смежности
    int minDistance; // Минимальное расстояние
    vector<int> minPath; // Кратчайший путь

    void update() {
        QWidget::update();
    }
};


int main(int argc, char** argv) {
    QApplication app(argc, argv);

    GraphWidget graphWidget;
    graphWidget.setGeometry(710, 290, 500, 500);
    graphWidget.show();

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(graphWidget.addVertexButton);
    layout->addWidget(graphWidget.removeVertexButton);
    layout->addWidget(graphWidget.addEdgeButton);
    layout->addWidget(graphWidget.removeEdgeButton);
    layout->addWidget(graphWidget.calculateButton);

    QWidget widget;
    widget.setLayout(layout);
    widget.setGeometry(1210, 290, 190, 160);
    widget.show();

    return app.exec();
}
