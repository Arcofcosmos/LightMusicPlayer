#ifndef MUSICLISTWIDGETDIALOG_H
#define MUSICLISTWIDGETDIALOG_H

#include <QDialog>

#include "musiclist.h"
#include "musiclistwidget.h"


namespace Ui {
class MusicListWidgetDialog;
}


class MusicListWidgetDialog : public QDialog
{
    Q_OBJECT

private:

    int     nums;          //歌曲数量
    bool    *selected_flag;     //歌曲是否被选中

public:
    explicit MusicListWidgetDialog(QWidget *parent = nullptr);

    //设定对话框展示出的歌单以及选择结果存放的地方
    void setMusicList(MusicList& ilist, bool *results);

    ~MusicListWidgetDialog();

private slots:
    void on_buttonBox_accepted();



    void on_buttonBox_rejected();

private:
    Ui::MusicListWidgetDialog *ui;
};

#endif // MUSICLISTWIDGETDIALOG_H
