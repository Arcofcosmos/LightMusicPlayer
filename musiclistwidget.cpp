#include "musiclistwidget.h"

MusicListWidget::MusicListWidget(QWidget *parent) : QListWidget(parent)
{

}


//刷新歌曲列表
void MusicListWidget::refresh()
{
    clear();
    musicList.addToListWidget(this);
}


//移除选中的歌曲
void MusicListWidget::removeMusic()
{
    //从歌单中移除
    int pos = this->currentRow();
    musicList.removeIndexMusic(pos);

    //从listWidget中移除
    QListWidgetItem *tempItem = this->takeItem(pos);   //NOTE:此处似乎已经可以移除item
    this->removeItemWidget(tempItem);
    delete tempItem;
}


//打开歌曲所在文件夹
void MusicListWidget::openFolderOfMusic()
{
    int pos = this->currentRow();
    musicList.openFolderOfIndexMusic(pos);
}


//显示歌曲详细信息
void MusicListWidget::showDetailInfo()
{
    int pos = this->currentRow();
    musicList.showMusicDetailInfo(pos);
}


//设置当前歌曲列表
void MusicListWidget::setMusicList(const MusicList myMusicList)
{
    musicList = myMusicList;
    this->refresh();
}


//设置当前播放歌曲列表
void MusicListWidget::setMusicList_Playing(const MusicList myMusciList)
{
    musicList = myMusciList;
    musicList.setSqlFlag(false);     //播放歌单不需要与数据库交互
    this->refresh();
}







