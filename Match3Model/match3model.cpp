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
        mb.m_index = i;
        mb.m_color = QColor(myColors()[rand() % myColors().size()]);
        m_board.append(mb);
    }
}

Match3model::~Match3model() { }

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

//void Match3model::boardValid(){

//}

//bool Match3model::gameOver(){
//    bool m_gameOver = true;

//    return m_gameOver;
//}

void Match3model::remove(int index){
    if(index < 0 || index >= m_board.count()){
        return;
    }
    beginRemoveRows(QModelIndex(), index, index);
    m_board.removeAt(index);
    endRemoveRows();
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
