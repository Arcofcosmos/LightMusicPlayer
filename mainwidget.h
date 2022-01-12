#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QPoint>
#include <QAction>
#include <QTimer>
#include <QMessageBox>


#include "lyricwidget.h"
#include "musiclist.h"
#include "tcpthread.h"

/***
stackWidget各个页面
0：当前播放
1：搜索音乐
2：我的喜欢
3：本地音乐
4：歌单
5：歌词
***/



namespace Ui {
class MainWidget;
}


class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget() override;

private:
    Ui::MainWidget *ui;

    TcpThread *tcp_thread;

    QMediaPlayer    *player;                   //当前的音乐播放器
    QMediaPlaylist  *playlist;               //当前的音乐播放列表

    int     musicLists_index = -1;          //表示当前歌单索引
    int     stackPageIndex = 0;             //当前多页面索引
    qint64  historyPosition = 0;                //上次播放的进度
    double  rotateAngle = 0;                    //图像旋转角度

    QTimer *imageTimer;
    QPixmap currentImage;                       //当前歌曲图片


//    QTimer *LuoQiTimer;
//    void    LuoQi();

    QVector<MusicList> musicLists;          //存放自定义的歌单

    void paintEvent(QPaintEvent *event) override;

    void    init_play();                    //播放设置的初始化
    void    init_UI();                      //一些ui控件的美化

    void    init_musicLists();              //初始化所有歌单
    //更新播放进度信息等
    void    updatePosition(int position);
    void    updateDuration(int duration);
    void    setPosition(int position);
    void    updateInfo();
    void    updatePlayBtn();

    void    init_settings();                //初始化程序的基础设置，如默认背景图片   
    void    initSqlite();                       //初始化数据库

    //系统托盘
    QSystemTrayIcon *mySystemTray;
    QAction *action_systemTray_play;         //系统托盘播放动作
    QAction *action_systemTray_playmode;        //系统托盘播放模式动作
    //响应系统托盘激活动作，如双击
    void    systemTray_activated(QSystemTrayIcon::ActivationReason reason);
    void    initSystemTray();                   //初始化系统托盘

    void    initMenuAction();                   //初始化右键菜单栏
    QMenu   *menu_currentPlay;                  //“当前播放”的右键菜单
    QMenu   *menu_localMusic;                   //“本地音乐”的右键菜单
    QMenu   *menu_favorMusic;                   //“我的喜欢”的右键菜单
    QMenu   *menu_musicListName;                //”歌单名“的右键菜单
    QMenu   *menu_musicList;                    //歌单列表的右键菜单
    QMenu   *menu_skinSwitch;                   //背景皮肤切换的右键菜单
    QMenu   *menu_searchedMusic;                //“搜索音乐”的右键菜单

    void    quitMusicPlayer();                  //退出音乐播放器

    void    musicList_refresh();            //刷新当前我的歌单显示的内容

    void    namelist_refresh();             //更新展示歌单名字的listwidget

    void setCirclePixmap(const QPixmap & circlepixmap, bool flag);

    void showSearchedMusic(QString keyWord);

protected:

    //窗口拖动时记录的起始点
    QPoint offset;
    /*重写Widget的一些方法*/
    //鼠标事件
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    //关闭时不退出，而是到系统托盘
    void closeEvent(QCloseEvent *event) override;
    //拖拽文件进入
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    //按键事件重写
    void keyPressEvent(QKeyEvent *event) override;



private slots:

    void imageRotate();

    /*部分右键菜单项对应的操作（即对应QAction连接的槽函数）*/
    void playlist_removeMusic();                //当前播放列表-右键菜单 移除歌曲
    void play_to_favor();                       //从当前播放添加到我喜欢
    void local_to_favor();                      //从本地音乐添加到我喜欢
    void local_to_playlist();                   //从本地音乐添加到当前播放列表
    void searched_to_favor();                   //从搜索的音乐添加到我的喜欢
    void searched_to_playlist();                //从搜索的音乐添加到播放列表
    void favor_to_playlist();                   //从我喜欢添加到当前播放列表
    void namelist_delete();                     //移除歌单
    void musiclist_removeMusic();               //从歌单展示列表移除歌曲
    void musiclist_to_favor();                  //从当前歌单添加到我喜欢
    void musiclist_to_playlist();               //从当前歌单添加到正在播放
    void background_to_default();               //换到默认背景
    void background_setting();                  //自定义背景

    //界面控件触发的槽函数
    void on_btnPre_clicked();
    void on_btnPlay_clicked();
    void on_btnPlayMode_clicked();
    void on_btnAddMusic_clicked();
    void on_btnVolume_clicked();
    void on_btnNext_clicked();
    //控件触发菜单的槽函数
    void on_curMusicWidget_customContextMenuRequested(const QPoint &pos);
    void on_favorMusicWidget_customContextMenuRequested(const QPoint &pos);
    void on_localMusicWidget_customContextMenuRequested(const QPoint &pos);
    void on_myMusicList_customContextMenuRequested(const QPoint &pos);
    void on_nameMusicList_customContextMenuRequested(const QPoint &pos);
    void on_btnCurMusic_clicked();
    void on_btnLocalMusic_clicked();
    void on_btnFavorMusic_clicked();
    void on_btnMin_clicked();
    void on_btnExit_clicked();
    void on_btnSkin_clicked();
    void on_btnAbout_clicked();

    void on_volumeSlider_valueChanged(int value);
    void on_btnAddMusicList_clicked();
    void on_curMusicWidget_doubleClicked(const QModelIndex &index);
    void on_favorMusicWidget_doubleClicked(const QModelIndex &index);
    void on_localMusicWidget_doubleClicked(const QModelIndex &index);
    void on_myMusicList_doubleClicked(const QModelIndex &index);
    void on_nameMusicList_doubleClicked(const QModelIndex &index);
    void on_btnAddMyMusicList_clicked();
    void on_btnAddFavorMusic_clicked();
    void on_btnTitle_clicked();
    void on_btnClearCur_clicked();
    void on_btnFavorClear_clicked();
    void on_btnFavorNeaten_clicked();
    void on_btnSortTitle_favor_clicked();
    void on_btnLocalNeaten_clicked();
    void on_btnLocalClear_clicked();
    void on_btnSortSinger_local_clicked();
    void on_btnSortSinger_favor_clicked();
    void on_btnSortDuration_favor_clicked();
    void on_btnMyMusicListClear_clicked();
    void on_btnMyMusicListNeaten_clicked();
    void on_btnSortDuration_local_clicked();
    void on_btnSortTitle_local_clicked();
    void on_btnSortDuration_my_clicked();
    void on_btnSortSinger_my_clicked();
    void on_btnSortTitle_my_clicked();
    void on_btnLyric_clicked();
    void on_searchedMusicWidget_customContextMenuRequested(const QPoint &pos);
    void on_searchedMusicWidget_doubleClicked(const QModelIndex &index);
};

#endif // MAINWIDGET_H
