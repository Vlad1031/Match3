#include "match3model.h"
#include <random>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>

Match3model::Match3model(QObject *parent) : QAbstractListModel(parent)
{
    m_roleNames[ColorRole] = "color";

    for(int i = 0; i < myColumns() * myRows(); i++){
        mBoard mb;
        mb.m_color = myColors()[rand() % myColors().size()];
        m_board.append(mb);
    }
    shaffle();
}

Match3model::~Match3model() { }

void Match3model::shaffle(){
    auto seed = chrono::system_clock::now().time_since_epoch().count();
    static mt19937 generator(seed);

    do{
        shuffle(m_board.begin(), m_board.end(), generator);
        emit dataChanged(createIndex(0, 0), createIndex(myColumns() * myRows(), 0));
    }while(!removeIndex.isEmpty());
}

int Match3model::myRows(){
    QFile file;
    file.setFileName(":size.json");
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Not JSON file";
    }

    QTextStream file_text(&file);
    QString jString;
    jString = file_text.readAll();
    file.close();
    QByteArray jByte = jString.toLocal8Bit();

    auto jDocument = QJsonDocument::fromJson(jByte);

    QJsonObject jObject = jDocument.object();
    QJsonValue rows = jObject.value("rows");

    return rows.toInt();
}

int Match3model::myColumns(){
    QFile file;
    file.setFileName(":size.json");
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Not JSON file";
    }

    QTextStream file_text(&file);
    QString jString;
    jString = file_text.readAll();
    file.close();
    QByteArray jByte = jString.toLocal8Bit();

    auto jDocument = QJsonDocument::fromJson(jByte);

    QJsonObject jObject = jDocument.object();
    QJsonValue columns = jObject.value("columns");

    return columns.toInt();
}
QList<QString> Match3model::myColors(){
    QFile file;
    file.setFileName(":size.json");
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() << "Not JSON file";
    }

    QTextStream file_text(&file);
    QString jString;
    jString = file_text.readAll();
    file.close();

    QByteArray jByte = jString.toLocal8Bit();
    auto jDocument = QJsonDocument::fromJson(jByte);
    QJsonObject jObject = jDocument.object();

    QJsonArray colors = jObject.value("colors").toArray();
    QList<QString> myColor;

    foreach(QJsonValue allColors, colors){
        myColor.append(allColors.toString());
    }
    return myColor;
}

bool Match3model::neighboring(int from, int to){
    if(from % myColumns() < myColumns() - 1 && from + 1 == to)
        return true;
    if(from % myColumns() > 0 && from - 1 == to)
        return true;
    if(from < (myColumns() * myRows() - myColumns()) && from + myColumns() == to)
        return true;
    if(from >= myColumns() && from - myColumns() == to)
        return true;
    return false;
}

void Match3model::move(int from, int to){
    if(neighboring(from, to)){
        m_board.move(from, to);
        if(from > to){
            beginMoveRows(QModelIndex(), from, from, QModelIndex(), to);
            endMoveRows();
            if(to < from && to < from - 1){
                beginMoveRows(QModelIndex(), to + 1, to + 1, QModelIndex(), from + 1);
                endMoveRows();
            }
            m_board.move(to + 1, from);
        }

        else{
            beginMoveRows(QModelIndex(), from, from, QModelIndex(), to + 1);
            endMoveRows();
            if(to > from && to > from + 1){
                beginMoveRows(QModelIndex(), to - 1, to - 1, QModelIndex(), from);
                endMoveRows();
            }
            m_board.move(to - 1, from);
        }
    }
}

bool Match3model::combinations(){
    for(int i = 0; i < myRows(); i++){
        for(int j = 0; j < myColumns() - 2; j++){
            if(m_board.at(j + 2 + myColumns() * i).m_color == m_board.at(j + myColumns() * i).m_color &&
                    m_board.at(j + myColumns() * i).m_color == m_board.at(j + 1 + myColumns() * i).m_color){
                for(int k = 0; k < 3; k++){
                    if(find(removeIndex.begin(), removeIndex.end(), j + k + myColumns() * i) == removeIndex.end()){
                        removeIndex.push_back(j + k + myColumns() * i);
                    }
                }
            }
        }
    }

    for(int i = 0; i < myColumns(); i++){
        for(int j = 0; j < myRows() - 2; j++){
            if(m_board.at(myColumns() * (j + 2) + i).m_color == m_board.at(myColumns() * j + i).m_color &&
                    m_board.at(myColumns() * j + i).m_color == m_board.at(myColumns() * (j + 1) + i).m_color){
                for(int k = 0; k < 3; k++){
                    if(find(removeIndex.begin(), removeIndex.end(), myColumns() * (j + k) + i) == removeIndex.end()){
                        removeIndex.push_back(myColumns() * (j + k) + i);
                    }
                }
            }
        }
    }
    qDebug() << removeIndex;
    return !removeIndex.empty();
}

void Match3model::drop_match(){
    while(!removeIndex.isEmpty()){
        beginRemoveRows(QModelIndex(), removeIndex[0], removeIndex[0]);
        m_board.removeAt(removeIndex[0]);
        endRemoveRows();

        beginInsertRows(QModelIndex(), removeIndex[0], removeIndex[0]);
        m_board.insert(removeIndex[0], mBoard(myColors()[rand() % myColors().size()]));
        endInsertRows();

        for(int i = removeIndex[0]; i > myColumns() - 1; i -= myColumns()){
            beginMoveRows(QModelIndex(), i, i, QModelIndex(), i - myColumns());
            endMoveRows();
            beginMoveRows(QModelIndex(), i - myColumns() + 1, i - myColumns() + 1, QModelIndex(), i + 1);
            endMoveRows();
            m_board.move(i - myColumns() + 1, i + 1);
        }

        removeIndex.pop_front();
    }
}

int Match3model::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_board.count();
}

QVariant Match3model::data(const QModelIndex &index, int role) const
{
    if(index.row() < 0 || index.row() > m_board.count()){
        return QVariant();
    }

    switch (role) {
    case ColorRole:
        return m_board.value(index.row()).m_color;
    }
    return QVariant();
}

QHash<int, QByteArray> Match3model::roleNames() const{
    return m_roleNames;
}
