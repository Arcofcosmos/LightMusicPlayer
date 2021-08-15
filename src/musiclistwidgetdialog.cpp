#include "musiclistwidgetdialog.h"
#include "ui_musiclistwidgetdialog.h"

MusicListWidgetDialog::MusicListWidgetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MusicListWidgetDialog)
{
    ui->setupUi(this);

    //设置歌曲可以鼠标点击多选
    ui->listWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);
}

MusicListWidgetDialog::~MusicListWidgetDialog()
{
    delete ui;
}


//添加歌曲到歌单中，设定选中的歌曲
void MusicListWidgetDialog::setMusicList(MusicList &ilist, bool *results)
{
    selected_flag = results;
    ui->listWidget->setIcon(QIcon(":/image/image/image/dialog-music.png"));
    ilist.addToListWidget(ui->listWidget);      //将要添加歌曲所在歌单放入界面中
    nums = ui->listWidget->count();          //nums是要添加的歌曲所在歌单的数量
}


void MusicListWidgetDialog::on_buttonBox_accepted()
{
    for(int i = 0; i < nums; i++)
    {
        //qDebug() << "song is selected" << endl;
        //查看每首歌是否被选中
        selected_flag[i] = ui->listWidget->item(i)->isSelected();

    }

    this->accept();      //对话框返回接受结果并隐藏
}



void MusicListWidgetDialog::on_buttonBox_rejected()
{
    this->reject();
}
