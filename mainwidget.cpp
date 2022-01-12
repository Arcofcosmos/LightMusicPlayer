#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QtSql>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QMouseEvent>
#include <QMediaPlayer>
#include <QSettings>
#include <QMediaPlaylist>
#include <QScrollBar>
#include <QPainter>

#include "musiclistwidgetdialog.h"
#include "music.h"
#include "myqss.h"


MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    imageTimer = new QTimer(this);
    connect(imageTimer, &QTimer::timeout, this, &MainWidget::imageRotate);

    tcp_thread = new TcpThread(this);
    tcp_thread->start();
    //connect(tcp_thread->socket, &QTcpSocket::readyRead, tcp_thread, &TcpThread::readMessage);          //接收信息

//    LuoQiTimer = new QTimer(this);
//    connect(LuoQiTimer, &QTimer::timeout, this, &MainWidget::LuoQi);

    this->setAcceptDrops(true);         //启动拖动事件，此处必须加上

    this->init_UI();                    //初始化一些UI控件设置

    this->initSqlite();                 //初始化数据库

    this->init_settings();              //初始化默认背景以及程序设置保存

    this->init_play();                  //初始化播放器及相关槽函数连接

    this->initMenuAction();             //初始化菜单项

    this->init_musicLists();            //初始化歌单

    this->initSystemTray();             //初始化系统托盘及其菜单

    //ui->curMusicWidget->setFocusPolicy(Qt::NoFocus);

    //LuoQiTimer->start(8000);
}

MainWidget::~MainWidget()
{
    delete ui;
}


//void MainWidget::LuoQi()
//{
//    QMessageBox::information(this, u8"罗同学你好", u8"想听歌？没看到上面有搜索框吗？先搜点想听的歌试试。");
//    LuoQiTimer->stop();
//}


void MainWidget::paintEvent(QPaintEvent *event)
{
    //需要添加以下代码，才能正常在主窗口Widget中显示背景图片（https://blog.csdn.net/xiejie0226/article/details/81165379）
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    QWidget::paintEvent(event);
}


//鼠标点击时间，点击界面某点隐藏音量条
void MainWidget::mousePressEvent(QMouseEvent *event)
{
    //实现点击界面中某点，音量条隐藏
    //鼠标坐标,相对于界面
    int x = event->pos().x();
    int y = event->pos().y();
    //音量条坐标
    int x_widget = ui->volumeSlider->geometry().x();
    int y_widget = ui->volumeSlider->geometry().y();
    int w = ui->volumeSlider->geometry().width();
    int h = ui->volumeSlider->geometry().height();

    //点击音量条以外的位置会隐藏音量条
    if(!(x>=x_widget && x<=x_widget+w && y>=y_widget && y<=y_widget+h))
    {
        ui->volumeSlider->hide();
    }

    //记录窗口移动的初始位置，offset实际就等于event->pos
    //offset = event->globalPos() - pos();      //全局坐标减界面坐标，pos为界面左上角相对于屏幕的坐标
    offset = event->pos();
//    qDebug() << "globapos: " << event->globalPos() << endl;
//    qDebug() << "pos: " << pos() << endl;
//    qDebug() << "event pos: " << event->pos() << endl;
    //qDebug() << "mouse pressed: " << offset << endl;
    //event->accept();           //NOTE:可以不需要
}


//鼠标移动事件,需要点击一下再移动就会触发
//窗口无状态栏无法拖动，重写该事件实现拖动
void MainWidget::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << "mouse move" << endl;
    //鼠标的坐标，相对于程序窗口
    int x = event->pos().x();
    int y = event->pos().y();
    //鼠标在这个范围内可拖动
    if((y < ui->titleLabel->geometry().height()) && (x < this->geometry().width()))
    {
        //不断变化的鼠标全局坐标减去点击时候固定的相对坐标
        move(event->globalPos() - offset);
//        qDebug() << "globalPos: " << event->globalPos() << endl;
//        qDebug() << "relult pos: " << event->globalPos() - offset << endl;
        //event->accept();
        setCursor(Qt::ClosedHandCursor);       //拖动时鼠标箭头变手型
    }
}


//鼠标释放事件
void MainWidget::mouseReleaseEvent(QMouseEvent *event)
{
    //offset = QPoint();              //刷新offset值
    //qDebug() << "new offset: " << offset << endl;
    event->accept();
    setCursor(Qt::ArrowCursor);                   //鼠标形状变箭头
}


//程序关闭事件,运行close()时会启动该事件
void MainWidget::closeEvent(QCloseEvent *event)
{
    //qDebug() << "close event" << endl;
    //最小化到托盘
    if(!mySystemTray->isVisible())
    {
        mySystemTray->show();
    }
    hide();
    //关于QCloseEvent的ignore和accept: https://blog.csdn.net/zhangbinsijifeng/article/details/51577862
    //QCloseEvent的ignore,accept与其它key,mouse事件的不一样
    event->ignore();          //注意此处应该使用ignore,ignore表示不退出程序,而accept表示退出
}


//文件拖拽事件
void MainWidget::dragEnterEvent(QDragEnterEvent *event)
{
    //qDebug() << "drag event" << endl;
    /*inline void acceptProposedAction() { drop_action = default_action; accept(); }
    而setDropAction用于设置非default_action，而后，你自己还需要再调用 accept()
    drop_action为文件拖拽的方式，比如移动拖拽，或者复制文件拖拽*/
    event->acceptProposedAction();
    //event->accept();
}


//拖拽释放事件
void MainWidget::dropEvent(QDropEvent *event)
{
    //qDebug() << "drop event" << endl;
    //获取拖拽文件的路径给本地音乐
    QList<QUrl> urls = event->mimeData()->urls();
    ui->localMusicWidget->musicList.addAllMusic(urls);
    ui->localMusicWidget->refresh();
    ui->stackedWidget->setCurrentIndex(2);//切换到本地音乐
    stackPageIndex = 2;
    event->accept();
}


//实现一下按键功能
void MainWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return)         //回车事件
    {
        if(!ui->searchLineEdit->text().isEmpty())
        {
            QString keyWords = ui->searchLineEdit->text();
//            QMessageBox::information(this, "加油！", "哈哈还没开发这功能呢！\n不过别急，生活不易，便愈发珍惜，\n抬起头一步一步积极往前走，\n车到山前必有路\n这里有你，并且不止有你。");
//            ui->searchLineEdit->setText("勇敢牛牛，不怕困难");
//            if(!tcp_thread->connectState())
//            {
//                tcp_thread->connectSevere();
//            }
//            tcp_thread->sendMessage(ui->searchLineEdit->text());
//            ui->searchLineEdit->clear();
            showSearchedMusic(keyWords);     //在缓存文件夹中搜索音乐并显示
        }
    }

    else if(event->modifiers() == Qt::AltModifier && event->key() == Qt::Key_Up)       //↑提高音量
    {
        if(!ui->volumeSlider->isVisible())
            ui->volumeSlider->setVisible(true);
        int currentVolume = player->volume();
        currentVolume += 10;
        this->on_volumeSlider_valueChanged(currentVolume);
    }

    else if(event->modifiers() == Qt::AltModifier && event->key() == Qt::Key_Down)     //↓降低音量
    {
        if(!ui->volumeSlider->isVisible())
            ui->volumeSlider->setVisible(true);
        int currentVolume = player->volume();
        currentVolume -= 10;
        this->on_volumeSlider_valueChanged(currentVolume);
    }

    else if (event->modifiers() == Qt::AltModifier && event->key() == Qt::Key_Space)
    {
        this->on_btnPlay_clicked();
    }
}


//显示搜索的歌曲到界面
void MainWidget::showSearchedMusic(QString keyWord)
{
    //创建缓存音乐路径
    QDir dir;
    if(!dir.exists("searchedMusic"))
    {
        dir.mkdir("searchedMusic");
    }
    dir = "searchedMusic";
    QStringList nameFilters;
    nameFilters << "*.mp3" << "*.lrc";
    //找到所有文件名
    QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);
    files = files.filter(keyWord);

    QDir tempDir;
    QList<QUrl> urlList;
    QString tempStr;
    for(int i = 0; i < files.size(); ++i)
    {
        tempDir = ("searchedMusic/" + files[i]);
        tempStr = tempDir.absolutePath();
        urlList.append(QUrl::fromLocalFile(tempStr));
    }

    ui->searchedMusicWidget->musicList.clear();
    ui->searchedMusicWidget->musicList.addAllMusic(urlList);
    ui->searchedMusicWidget->refresh();
    ui->stackedWidget->setCurrentIndex(1);       //切换到本地音乐页面
    stackPageIndex = 1;

//    QFileDialog fileDialog(this);
//    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
//    //setFileMode设置了添加时文件选中情况，ExistingFiles支持多选和ctrl+A
//    fileDialog.setFileMode(QFileDialog::ExistingFiles);
//    fileDialog.setWindowTitle("添加本地音乐（注：自动过滤，按下\"Ctrl+A\"全选添加即可；不支持添加文件夹）");
//    //设置mime类型文件过滤器，mime类型为描述文件或字节流格式和性质等的类型
//    //关于通用的mime类型：https://www.cnblogs.com/xiaohi/p/6550133.html
//    QStringList list;
//    list<<"application/octet-stream";            //该格式类型会显示所有文件
//    fileDialog.setMimeTypeFilters(list);
//    fileDialog.setDirectory("searchedMusic");
//    fileDialog.exec();
//    QList<QUrl> urls=fileDialog.selectedUrls();
//    ui->searchedMusicWidget->musicList.addAllMusic(urls);
//    ui->searchedMusicWidget->refresh();
//    ui->stackedWidget->setCurrentIndex(1);       //切换到本地音乐页面
//    stackPageIndex = 1;
}



//一些ui控件的美化
void MainWidget::init_UI()
{
    //窗口设置圆角后，会出现留白，需要添加以下代码
    setAttribute(Qt::WA_TranslucentBackground, true);
    //去除标题栏
    //setWindowFlags(Qt::FramelessWindowHint);
    //设置图边框，并且点击任务栏图标可以让其最小化
    setWindowFlags(Qt::FramelessWindowHint |Qt::WindowSystemMenuHint|Qt::WindowMinimizeButtonHint);

    //UI初始化（.ui文件中无法完成的设置，这里补上）
    ui->volumeSlider->hide();
    ui->curMusicWidget->verticalScrollBar()->setStyleSheet(ListWidgetStyle());
    ui->localMusicWidget->verticalScrollBar()->setStyleSheet(ListWidgetStyle());
    ui->favorMusicWidget->verticalScrollBar()->setStyleSheet(ListWidgetStyle());
    ui->nameMusicList->verticalScrollBar()->setStyleSheet(ListWidgetStyle());
    ui->myMusicList->verticalScrollBar()->setStyleSheet(ListWidgetStyle());
    ui->curMusicWidget->setIcon(QIcon(":/image/image/image/music.png"));
    ui->localMusicWidget->setIcon(QIcon(":/image/image/image/music-file.png"));
    ui->favorMusicWidget->setIcon(QIcon(":/image/image/image/like.png"));
    ui->myMusicList->setIcon(QIcon(":/image/image/image/MusicListItem.png"));
    ui->searchedMusicWidget->setIcon(QIcon(":/image/image/image/searched_music.png"));
//    QPixmap noMusicImage = QPixmap::fromImage(QImage(":/image/image/image/non-music.png"));
//    this->setCirclePixmap(noMusicImage, false);
}


//毫秒转换为时间字符串
QString convertTimeFormat(int timeMilliSeconds)
{
    int seconds = timeMilliSeconds/1000;
    int minutes = seconds / 60;
    seconds -= minutes * 60;
    //关于arg的参数https://blog.csdn.net/Fighting_YoungMan/article/details/70611052
    return QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0'));
}


//播放设置的初始化
void MainWidget::init_play()
{
    player = new QMediaPlayer(this);
    playlist = new QMediaPlaylist(this);
    //播放列表的歌曲播放顺序设置，Loop为整个歌单顺序循环播放
    playlist->setPlaybackMode(QMediaPlaylist::Loop);
    player->setPlaylist(playlist);
    player->setVolume(50);

    connect(ui->positionSlider, &QAbstractSlider::valueChanged, this, &MainWidget::setPosition);
    connect(player, &QMediaPlayer::positionChanged, this, &MainWidget::updatePosition);
    connect(player, &QMediaPlayer::durationChanged, this, &MainWidget::updateDuration);
    connect(player, &QMediaPlayer::metaDataAvailableChanged, this, &MainWidget::updateInfo);
    connect(player, &QMediaPlayer::stateChanged, this, &MainWidget::updatePlayBtn);

    QSettings settings("./LightMusicPlayer.ini", QSettings::IniFormat);
    settings.setIniCodec("UTF8");           //设置UTF8编码能使settings读取设置中的中文
    QUrl historyMusic = settings.value("historyMusic").toUrl();       //上次退出时播放的音乐
    historyPosition = settings.value("historyPosition").toInt();     //上次退出播放的进度
    if(!historyMusic.isEmpty())
    {
        ui->curMusicWidget->musicList.addSingleMusic(Music(historyMusic));
        ui->curMusicWidget->musicList.addToPlayList(playlist);
        ui->curMusicWidget->refresh();
        //qDebug() << curPosition << endl;
        ui->stackedWidget->setCurrentIndex(0);
        //player->play();
        //player->setPosition(curPosition);
    }
}


//播放更新进度条
void MainWidget::updatePosition(int position)
{
    //qDebug() << "currentPostion: " << position << endl;
    ui->positionSlider->setValue(position);
    ui->positionLabel->setText(convertTimeFormat(position) + "/" + convertTimeFormat(player->duration()));
    ui->lyricWidget->showLyricToWidget(position);       //NOTE:此处修改歌词显示
}


//切换歌曲或者播放歌曲被移除，更新信息
void MainWidget::updateDuration(int duration)
{
    ui->positionSlider->setRange(0, duration);      //设置滑动条长度
    ui->positionSlider->setEnabled(duration > 0);           //歌曲时间大于0就让滑动条能够滑动
    //设置滑动条每次点击的滑动步长
    ui->positionSlider->setPageStep(duration / 10);

    if(duration <= 0)           //当前无音乐播放
    {
        ui->infoLabel->setText(u8"KEEP CALM AND LISTEN TO YOUR HEART");        //更换默认信息
        mySystemTray->setToolTip(u8"LightMusicPlayer · By TuZhou");             //托盘tip信息更换默认
        QImage image(":/image/image/image/non-music.png");
        ui->musicImageLabel->setPixmap(QPixmap::fromImage(image));              //歌曲图片更换默认
        //清空界面上的歌词
        ui->musicTitleLabel->setText("");
        ui->musicAlbumLabel->setText("");
        ui->musicAuthorLabel->setText("");
        ui->lyricWidget->clearLyric();
    }

//    int pos = playlist->currentIndex();
//    qDebug() << "pos: " << pos;
//    ui->curMusicWidget->musicList.musicList[pos].duration = player->duration();
//    ui->curMusicWidget->refresh();
}


//根据播放进度条设置播放的进度
void MainWidget::setPosition(int position)
{
    // qAbs取绝对值，当进度条变化值超过99才会更新进度
    if (qAbs(player->position() - position) > 99)
    {
        player->setPosition(position);
        //qDebug() << "setPosition: " <<  position << endl;
    }
}


//歌曲改变，歌曲的显示信息也需改变
void MainWidget::updateInfo()
{
    if (player->isMetaDataAvailable())
    {
        //qDebug() << "remove" << endl;
        if(imageTimer->isActive())
        {
            //qDebug() << "stop timer" << endl;
            imageTimer->stop();
        }

        //歌曲信息
        QString info = "";
        QString author = player->metaData(QStringLiteral("Author")).toStringList().join(",");
        info.append(author);
        QString title = player->metaData(QStringLiteral("Title")).toString();
        QString albumTitle = player->metaData(QStringLiteral("AlbumTitle")).toString();
        info.append(" - "+title);
        info.append(" ["+convertTimeFormat(player->duration())+"]");
        ui->infoLabel->setText(info);

        //切换托盘显示的tip信息
        mySystemTray->setToolTip("正在播放：" + info);

        //设置歌曲封面图片
        //封面图片（应获取"ThumbnailImage" From: https://www.zhihu.com/question/36859497）
        //value将QVariant类型值转换成QImage类型
        QPixmap picImage= player->metaData(QStringLiteral("ThumbnailImage")).value<QPixmap>();
        //QPixmap pixmap = QPixmap::fromImage(picImage);
        if(picImage.isNull())
            picImage = QPixmap::fromImage(QImage(":/image/image/image/non-music.png"));
        //ui->musicImageLabel->setPixmap(picImage);
        this->setCirclePixmap(picImage, true);        //显示圆形图片
        //setScaledContents设置缩放label让其刚好填充图片，使图片看起来没有多余的边角
        ui->musicImageLabel->setScaledContents(true);

        //改变正在播放歌曲的图标
        //NOTE:此处是否需要
        for(int i=0; i<playlist->mediaCount(); i++)
        {
            QListWidgetItem *p = ui->curMusicWidget->item(i);    //TODO:此处浪费时间可改进
            p->setIcon(ui->curMusicWidget->getIcon());
        }

        int index = playlist->currentIndex();
        QListWidgetItem *p = ui->curMusicWidget->item(index);
        p->setIcon(QIcon(":/image/image/image/music-playing.png"));

        //歌词界面显示的信息
        ui->musicTitleLabel->setText(title);
        ui->musicAlbumLabel->setText(u8"专辑：" + albumTitle);
        ui->musicAuthorLabel->setText(u8"歌手：" + author);

        //qDebug() << ui->curMusicWidget->musicList.musicList[index].getLyricUrl();
        //解析歌词
        ui->lyricWidget->processLyric(ui->curMusicWidget->musicList.musicList[index].getLyricUrl());
    }
}


//设置播放按钮切换
void MainWidget::updatePlayBtn()
{
    if(player->state()==QMediaPlayer::PlayingState)
    {
        ui->btnPlay->setStyleSheet(PlayStyle());
        action_systemTray_play->setIcon(QIcon(":/image/image/image/pause2.png"));
        action_systemTray_play->setText(u8"暂停");
    }
    else
    {
        ui->btnPlay->setStyleSheet(PauseStyle());
        action_systemTray_play->setIcon(QIcon(":/image/image/image/play2.png"));
        action_systemTray_play->setText(u8"播放");
    }
}


//退出程序
void MainWidget::quitMusicPlayer()
{
    //qDebug() << "quit" << endl;
    if(player->state() == QMediaPlayer::PlayingState)
    {
        historyPosition = player->position();
        QSettings settings("./LightMusicPlayer.ini", QSettings::IniFormat);
        settings.setIniCodec("UTF8");           //设置UTF8编码能使settings读取设置中的中文
        settings.setValue("historyMusic", ui->curMusicWidget->musicList.musicList[playlist->currentIndex()].url);
        settings.setValue("historyPosition", historyPosition);
    }
    QCoreApplication::exit(0);
    //this->close();
}


//初始化系统托盘
void MainWidget::initSystemTray()
{
    mySystemTray = new QSystemTrayIcon(this);
    mySystemTray->setIcon(QIcon(":/image/image/myImage/music.png"));
    mySystemTray->setToolTip(u8"LightMusicPlayer · By TuZhou");
    //系统托盘激活后发送信号，激活方式可以是点击双击等
    connect(mySystemTray,&QSystemTrayIcon::activated,this,&MainWidget::systemTray_activated);
    //添加菜单项
    QAction *action_systemTray_pre = new QAction(QIcon(":/image/image/image/pre2.png"), u8"上一首");
    connect(action_systemTray_pre, &QAction::triggered, this, &MainWidget::on_btnPre_clicked);
    action_systemTray_play = new QAction(QIcon(":/image/image/image/play2.png"), u8"播放");
    connect(action_systemTray_play, &QAction::triggered, this, &MainWidget::on_btnPlay_clicked);
    QAction *action_systemTray_next = new QAction(QIcon(":/image/image/image/next2.png"), u8"下一首");
    connect(action_systemTray_next, &QAction::triggered, this, &MainWidget::on_btnNext_clicked);
    action_systemTray_playmode = new QAction(QIcon(":/image/image/image/loop2.png"), u8"循环播放");
    connect(action_systemTray_playmode, &QAction::triggered, this, &MainWidget::on_btnPlayMode_clicked);
    QAction *action_systemTray_quit = new QAction(QIcon(":/image/image/image/exit.png"), u8"退出应用");
    connect(action_systemTray_quit, &QAction::triggered, this, &MainWidget::quitMusicPlayer);

    QMenu *pContextMenu = new QMenu(this);
    pContextMenu->addAction(action_systemTray_pre);
    pContextMenu->addAction(action_systemTray_play);
    pContextMenu->addAction(action_systemTray_next);
    pContextMenu->addAction(action_systemTray_playmode);
    pContextMenu->addAction(action_systemTray_quit);
    mySystemTray->setContextMenu(pContextMenu);          //系统托盘可直接设置右键菜单
    mySystemTray->show();
}


//系统托盘激活后做出反应
void MainWidget::systemTray_activated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    //对托盘双击
    case QSystemTrayIcon::DoubleClick:
        //显/隐主界面
        if(isHidden())
        {
            show();
        }else
        {
            hide();
        }
        break;
    default:
        break;
    }
}


//初始化右键菜单
void MainWidget::initMenuAction()
{
    //当前播放列表菜单初始化
    //setContextMenuPolicy设置菜单上下文策略，如鼠标右键触发菜单，菜单如何展现的
    //CustomContextMenu策略能够显示菜单同时并能发送信号
    //对控件添加customContextMenuRequested信号来激活菜单
    //QAction代表动作，能够嵌入到部件中通过触发反应相应动作，triggered代表触发信号，例如对部件点击，双击等能够触发信号
    ui->curMusicWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    QAction *action_playList_delete = new QAction(QIcon(":/image/image/image/remove.png"),u8"移除");
    connect(action_playList_delete, &QAction::triggered, this, &MainWidget::playlist_removeMusic);
    QAction *action_playList_openFolder = new QAction(QIcon(":/image/image/image/music-dir.png"),u8"打开所在文件夹");
    connect(action_playList_openFolder, &QAction::triggered, ui->curMusicWidget, &MusicListWidget::openFolderOfMusic);
    QAction *action_playlist_detail=new QAction(QIcon(":/image/image/image/detail.png"),u8"歌曲详情");
    connect(action_playlist_detail,&QAction::triggered,ui->curMusicWidget,&MusicListWidget::showDetailInfo);
    QAction *action_play_to_favor=new QAction(QIcon(":/image/image/image/To-like.png"),u8"添加到我喜欢");
    connect(action_play_to_favor,&QAction::triggered,this,&MainWidget::play_to_favor);
    //addAction将定义的动作嵌入到菜单中
    menu_currentPlay = new QMenu(this);
    menu_currentPlay->addAction(action_playList_delete);
    menu_currentPlay->addAction(action_playList_openFolder);
    menu_currentPlay->addAction(action_playlist_detail);
    menu_currentPlay->addAction(action_play_to_favor);

    //“本地音乐”列表右键菜单初始化
    ui->localMusicWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    QAction *action_locallist_delete = new QAction(QIcon(":/image/image/image/remove.png"),u8"移除");
    connect(action_locallist_delete,&QAction::triggered,ui->localMusicWidget,&MusicListWidget::removeMusic);
    QAction *action_locallist_showfile = new QAction(QIcon(":/image/image/image/music-dir.png"),u8"打开所在文件夹");
    connect(action_locallist_showfile,&QAction::triggered,ui->localMusicWidget,&MusicListWidget::openFolderOfMusic);
    QAction *action_locallist_detail = new QAction(QIcon(":/image/image/image/detail.png"),u8"歌曲详情");
    connect(action_locallist_detail,&QAction::triggered,ui->localMusicWidget,&MusicListWidget::showDetailInfo);
    QAction *action_local_to_favor = new QAction(QIcon(":/image/image/image/To-like.png"),u8"添加到我喜欢");
    connect(action_local_to_favor,&QAction::triggered,this,&MainWidget::local_to_favor);
    QAction *action_local_to_playlist = new QAction(QIcon(":/image/image/image/To-playlist.png"),u8"添加到当前播放列表");
    connect(action_local_to_playlist,&QAction::triggered,this,&MainWidget::local_to_playlist);

    menu_localMusic = new QMenu(this);
    menu_localMusic->addAction(action_locallist_delete);
    menu_localMusic->addAction(action_locallist_showfile);
    menu_localMusic->addAction(action_locallist_detail);
    menu_localMusic->addAction(action_local_to_favor);
    menu_localMusic->addAction(action_local_to_playlist);

    //“我喜欢”列表右键菜单初始化
    ui->favorMusicWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    QAction *action_favorlist_delete = new QAction(QIcon(":/image/image/image/remove.png"),u8"移除");
    connect(action_favorlist_delete, &QAction::triggered, ui->favorMusicWidget, &MusicListWidget::removeMusic);
    QAction *action_favorlist_showfile = new QAction(QIcon(":/image/image/image/music-dir.png"),u8"打开所在文件夹");
    connect(action_favorlist_showfile, &QAction::triggered, ui->favorMusicWidget, &MusicListWidget::openFolderOfMusic);
    QAction *action_favorlist_detail = new QAction(QIcon(":/image/image/image/detail.png"),u8"歌曲详情");
    connect(action_favorlist_detail, &QAction::triggered, ui->favorMusicWidget, &MusicListWidget::showDetailInfo);
    QAction *action_favor_to_playlist=new QAction(QIcon(":/image/image/image/To-playlist.png"),u8"添加到当前播放列表");
    connect(action_favor_to_playlist,&QAction::triggered,this,&MainWidget::favor_to_playlist);

    menu_favorMusic = new QMenu(this);
    menu_favorMusic->addAction(action_favorlist_delete);
    menu_favorMusic->addAction(action_favorlist_showfile);
    menu_favorMusic->addAction(action_favorlist_detail);
    menu_favorMusic->addAction(action_favor_to_playlist);

    //“我的歌单”列表右键菜单初始化
    ui->nameMusicList->setContextMenuPolicy(Qt::CustomContextMenu);
    QAction *action_namelist_delete=new QAction(QIcon(":/image/image/image/remove.png"),u8"删除");
    connect(action_namelist_delete,&QAction::triggered,this,&MainWidget::namelist_delete);

    menu_musicListName=new QMenu(this);
    menu_musicListName->addAction(action_namelist_delete);

    //歌单展示列表右键菜单初始化
    ui->myMusicList->setContextMenuPolicy(Qt::CustomContextMenu);
    QAction *action_musiclist_delete = new QAction(QIcon(":/image/image/image/remove.png"), u8"移除");
    connect(action_musiclist_delete, &QAction::triggered, this, &MainWidget::musiclist_removeMusic);
    QAction *action_musiclist_showfile = new QAction(QIcon(":/image/image/image/music-dir.png"), u8"打开所在文件夹");
    connect(action_musiclist_showfile, &QAction::triggered, ui->myMusicList, &MusicListWidget::openFolderOfMusic);
    QAction *action_musiclist_detail = new QAction(QIcon(":/image/image/image/detail.png"), u8"歌曲详情");
    connect(action_musiclist_detail, &QAction::triggered, ui->myMusicList, &MusicListWidget::showDetailInfo);
    QAction *action_musiclist_to_favor = new QAction(QIcon(":/image/image/image/To-like.png"), u8"添加到我喜欢");
    connect(action_musiclist_to_favor, &QAction::triggered, this, &MainWidget::musiclist_to_favor);
    QAction *action_musiclist_to_playlist = new QAction(QIcon(":/image/image/image/To-playlist.png"), u8"添加到当前播放列表");
    connect(action_musiclist_to_playlist, &QAction::triggered, this, &MainWidget::musiclist_to_playlist);

    menu_musicList = new QMenu(this);
    menu_musicList->addAction(action_musiclist_delete);
    menu_musicList->addAction(action_musiclist_showfile);
    menu_musicList->addAction(action_musiclist_detail);
    menu_musicList->addAction(action_musiclist_to_favor);
    menu_musicList->addAction(action_musiclist_to_playlist);


    //“搜索音乐”列表右键菜单初始化
    ui->searchedMusicWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    QAction *action_searchedlist_delete = new QAction(QIcon(":/image/image/image/remove.png"),u8"移除");
    connect(action_searchedlist_delete,&QAction::triggered,ui->searchedMusicWidget,&MusicListWidget::removeMusic);
    QAction *action_searchedlist_showfile = new QAction(QIcon(":/image/image/image/music-dir.png"),u8"打开所在文件夹");
    connect(action_searchedlist_showfile,&QAction::triggered,ui->searchedMusicWidget,&MusicListWidget::openFolderOfMusic);
    QAction *action_searchedlist_detail = new QAction(QIcon(":/image/image/image/detail.png"),u8"歌曲详情");
    connect(action_searchedlist_detail,&QAction::triggered,ui->searchedMusicWidget,&MusicListWidget::showDetailInfo);
    QAction *action_searched_to_favor = new QAction(QIcon(":/image/image/image/To-like.png"),u8"添加到我喜欢");
    connect(action_searched_to_favor,&QAction::triggered,this,&MainWidget::searched_to_favor);
    QAction *action_searched_to_playlist = new QAction(QIcon(":/image/image/image/To-playlist.png"),u8"添加到当前播放列表");
    connect(action_searched_to_playlist,&QAction::triggered,this,&MainWidget::searched_to_playlist);

    menu_searchedMusic = new QMenu(this);
    menu_searchedMusic->addAction(action_searchedlist_delete);
    menu_searchedMusic->addAction(action_searchedlist_showfile);
    menu_searchedMusic->addAction(action_searchedlist_detail);
    menu_searchedMusic->addAction(action_searched_to_favor);
    menu_searchedMusic->addAction(action_searched_to_playlist);

    //“更换背景皮肤”的菜单项，更换皮肤菜单点击触发，不需要鼠标右键
    QAction *action_backgroud_to_default = new QAction(QIcon(":/image/image/image/default.png"),u8"更换到默认背景");
    connect(action_backgroud_to_default,&QAction::triggered,this,&MainWidget::background_to_default);
    QAction *action_backgroud_setting=new QAction(QIcon(":/image/image/image/setting.png"),u8"自定义背景");
    connect(action_backgroud_setting,&QAction::triggered,this,&MainWidget::background_setting);

    menu_skinSwitch=new QMenu(this);
    menu_skinSwitch->addAction(action_backgroud_to_default);
    menu_skinSwitch->addAction(action_backgroud_setting);
}


//初始化数据库https://www.cnblogs.com/xia-weiwen/p/6806709.html
void MainWidget::initSqlite()
{
    //创建数据库
    QSqlDatabase sqlite_database;
    //检查是否存在该数据库连接，qt_sql_default_connection为数据库默认连接名
    if (QSqlDatabase::contains("qt_sql_default_connection"))
    {
        sqlite_database = QSqlDatabase::database("qt_sql_default_connection");
    }

    else      //没有连接创建数据库连接
    {
        //添加数据库驱动为sqlite，参数2为数据库连接名，不写则默认
        sqlite_database = QSqlDatabase::addDatabase("QSQLITE");
        sqlite_database.setDatabaseName("Music.db");            //设置数据库文件名
        if(!sqlite_database.open())         //数据库打开失败
        {
            //lastError返回数据库最后产生的错误信息，databaseText将错误信息转换成QString
            QMessageBox::warning(this, u8"数据库Music.db打开失败", sqlite_database.lastError().databaseText());
            exit(-1);
        }
    }

    //创建表
    QSqlQuery query1;
    //查询该表是否存在，sqlite_master为sqlite系统表，所有表的创建能在该表中查询到
    query1.exec(QString("select count(*) from sqlite_master where type='table' and name='%1'").arg("MusicInfo"));
    //exec执行后经过next才会到达查询到的第一条信息
    if(query1.next())
    {
        if(query1.value(0).toInt() == 0)         //查询结果为0
        {
            QSqlQuery sql_query;
            //创建歌曲信息表
            QString create_sql = "create table MusicInfo (name varchar(30), url varchar(200), singer varchar(50), songTitle varchar(50), duration bigint, albumTitle varchar(50), audioBitRate int)";
            sql_query.prepare(create_sql);
            sql_query.exec();
        }
    }

    QSqlQuery query2;
    query2.exec(QString("select count(*) from sqlite_master where type='table' and name='%1'").arg("MusicLists"));
    if(query2.next())
    {
        if(query2.value(0).toInt() == 0)
        {
            QSqlQuery sql_query;
            //创建歌单表
            QString create_sql = "create table MusicLists (name varchar(30))";
            sql_query.prepare(create_sql);
            sql_query.exec();
        }
    }
}


//初始化所有歌单
void MainWidget::init_musicLists()
{
    //初始化本地音乐
    ui->localMusicWidget->musicList.setName("LocalMusic");
    ui->localMusicWidget->musicList.readSqlMusic();
    ui->localMusicWidget->refresh();          //刷新显示歌曲

    //初始化我喜欢的音乐
    ui->favorMusicWidget->musicList.setName("FavorMusic");
    ui->favorMusicWidget->musicList.readSqlMusic();
    ui->favorMusicWidget->refresh();

    //从数据库中恢复自定义歌单
    QSqlQuery sql_query;
    QString select_sql = "select name from MusicLists";
    sql_query.prepare(select_sql);
    if(sql_query.exec())
    {
        while(sql_query.next())
        {
            QString musicListName=sql_query.value(0).toString();
            MusicList tempList;
            tempList.setName(musicListName);
            tempList.readSqlMusic();
            musicLists.push_back(tempList);
        }
    }

    namelist_refresh();
}


//将歌曲封面变成圆形显示
void MainWidget::setCirclePixmap(const QPixmap & circlepixmap, bool flag)
{
    if(flag)           //歌曲图片
    {
        QPixmap pixmap(ui->musicImageLabel->width(), ui->musicImageLabel->height());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        QPainterPath path;
        path.addEllipse(0, 0, ui->musicImageLabel->width(),ui->musicImageLabel->height());
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, ui->musicImageLabel->width(),ui->musicImageLabel->height(), circlepixmap);
        //ui->musicImageLabel->setPixmap(pixmap);
        currentImage = pixmap;
        ui->musicImageLabel->setPixmap(currentImage);
        imageTimer->start(100);
    }

    else        //无音乐图片
    {
        QPixmap pixmap(ui->musicImageLabel->width(), ui->musicImageLabel->height());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        QPainterPath path;
        path.addEllipse(0, 0, ui->musicImageLabel->width(),ui->musicImageLabel->height());
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, ui->musicImageLabel->width(),ui->musicImageLabel->height(), circlepixmap);
        ui->musicImageLabel->setPixmap(pixmap);
    }
}


//实现图片旋转
void MainWidget::imageRotate()
{
    //qDebug() << "旋转" << endl;
    QMatrix matrix;
    matrix.rotate(rotateAngle);
    //QPixmap orig = currentImage;
    QPixmap rotated = currentImage.transformed(matrix, Qt::SmoothTransformation);
    ui->musicImageLabel->setPixmap(rotated);
    //this->setCirclePixmap(rotated);
    //rotateStep_ %= 2;
    rotateAngle += 2.2;
}


//设置刷新展示歌单名字的列表
void MainWidget::namelist_refresh()
{
    //先清空数据库中歌单表然后再重新添加
    QSqlQuery sql_query;
    QString delete_sql = "delete from MusicLists";
    sql_query.prepare(delete_sql);
    sql_query.exec();
    for(auto i = 0; i < musicLists.size(); i++)
    {
        QSqlQuery sql_query2;
        QString insert_sql = "insert into MusicLists values (?)";
        sql_query2.prepare(insert_sql);
        sql_query2.addBindValue(musicLists[i].getName());
        sql_query2.exec();
    }

    ui->nameMusicList->clear();     //先清空歌单名字列表
    //给歌单名字列表添加歌单名
    for(auto i = 0; i < musicLists.size(); i++)
    {
        QListWidgetItem *tempItem = new QListWidgetItem();
        tempItem->setIcon(QIcon(":/image/image/myImage/007-music.png"));
        tempItem->setText(musicLists[i].getName());
        ui->nameMusicList->addItem(tempItem);
    }
}


//刷新当前歌单显示的内容
void MainWidget::musicList_refresh()
{
    if(-1 != musicLists_index)
    {
        //显示歌单名
        ui->MyMusicListLabel->setText(u8"歌单 - " + musicLists[musicLists_index].getName());
        //显示歌单内容
        ui->myMusicList->setMusicList_Playing(musicLists[musicLists_index]);
    }
}


//初始化程序的默认设置，如程序默认背景图片
void MainWidget::init_settings()
{
    //创建保存设置的文件，IniFomat代表文件为.ini格式
    QSettings settings("./LightMusicPlayer.ini", QSettings::IniFormat);
    settings.setIniCodec("UTF8");           //设置UTF8编码能使settings读取设置中的中文
    //取出上一次使用的背景图片路径
    QString backImagePath = settings.value("backImageUrl1").toString();      //取出设置中名字为backImageUrl对应的值
    QImage backImage(backImagePath);

    //存储默认图片的路径
    QString defaultPath = settings.value("defaultImage").toString();
    if(defaultPath.isEmpty())
    {
        defaultPath = ":/image/image/background/back1.jpg";
        settings.setValue("defaultImage", defaultPath);
    }

    if(!backImagePath.isEmpty() && !backImage.isNull())
    {
        //此处必须添加setObjectName否则无背景图片
        //参考：https://www.cnblogs.com/sagerking/p/13986810.html
        this->setObjectName("Widget");
        setStyleSheet(QString("#Widget{"
                              "border-radius:10px;"
                              "border-image: url(%1);}").arg(backImagePath));
    }
    else
    {
        backImagePath = ":/image/image/background/back1.jpg";
        settings.setValue("backImageUrl1", backImagePath);
        this->setObjectName("Widget");
        setStyleSheet(QString("#Widget{"
                              "border-radius:10px;"               //表示边框圆角弧度
                              "border-image: url(%1);}").arg(backImagePath));
    }
}


//从播放列表中移除歌曲
void MainWidget::playlist_removeMusic()
{
    if(imageTimer->isActive())
    {
        imageTimer->stop();
    }

    int removed_pos = ui->curMusicWidget->currentRow();   //要删除的歌曲位置
    int playing_pos = playlist->currentIndex();           //播放列表中正在播放的位置
    ui->curMusicWidget->removeMusic();                    //先使选中歌曲从列表中消失

    if(removed_pos < playing_pos)         //删除的索引在播放的前面
    {
        QMediaPlayer::State preState = player->state();       //记录从播放列表中删除前歌曲播放状态
        int position = player->position();
        playing_pos--;            //播放的歌曲位置前面有歌曲被删，现在其位置改变
        playlist->removeMedia(removed_pos);             //此时正式在播放列表中删除选中歌曲
        playlist->setCurrentIndex(playing_pos);         //重新调整播放歌曲的位置
        player->setPosition(position);              //列表切换后必须重新调整播放进度，否则重头开始
        ui->positionSlider->setValue(position);
        //NOTE:此处去除if判断
    }

    else
    {
        playlist->removeMedia(removed_pos);
    }
}


//将当前播放中的歌曲加入我的喜欢中
void MainWidget::play_to_favor()
{
    int pos = ui->curMusicWidget->currentRow();
    ui->favorMusicWidget->musicList.addSingleMusic(ui->curMusicWidget->musicList.getIndexMusic(pos));
    ui->favorMusicWidget->refresh();
}


//从本地音乐中加入歌曲到我的喜欢中
void MainWidget::local_to_favor()
{
    int pos = ui->localMusicWidget->currentRow();
    ui->favorMusicWidget->musicList.addSingleMusic(ui->localMusicWidget->musicList.getIndexMusic(pos));
    ui->favorMusicWidget->refresh();
}





//将本地音乐放入当前播放列表中
void MainWidget::local_to_playlist()
{
    int pos = ui->localMusicWidget->currentRow();
    Music temp = ui->localMusicWidget->musicList.getIndexMusic(pos);
    ui->curMusicWidget->musicList.addSingleMusic(temp);            //添加到播放列表界面中
    ui->curMusicWidget->refresh();
    playlist->addMedia(temp.getUrl());              //添加到播放器中
}


//从搜索音乐中加入歌曲到我的喜欢中
void MainWidget::searched_to_favor()
{
    int pos = ui->searchedMusicWidget->currentRow();
    ui->favorMusicWidget->musicList.addSingleMusic(ui->searchedMusicWidget->musicList.getIndexMusic(pos));
    ui->favorMusicWidget->refresh();
}



//将搜索音乐放入当前播放列表中
void MainWidget::searched_to_playlist()
{
    int pos = ui->searchedMusicWidget->currentRow();
    Music temp = ui->searchedMusicWidget->musicList.getIndexMusic(pos);
    ui->curMusicWidget->musicList.addSingleMusic(temp);            //添加到播放列表界面中
    ui->curMusicWidget->refresh();
    playlist->addMedia(temp.getUrl());              //添加到播放器中
}


//将我的喜欢的音乐放入当前播放列表中
void MainWidget::favor_to_playlist()
{
    int pos = ui->favorMusicWidget->currentRow();
    Music temp = ui->favorMusicWidget->musicList.getIndexMusic(pos);
    ui->curMusicWidget->musicList.addSingleMusic(temp);            //添加到播放列表界面中
    ui->curMusicWidget->refresh();
    playlist->addMedia(temp.getUrl());              //添加到播放器中
}


//从歌单名字列表中移除歌单
void MainWidget::namelist_delete()
{
    int pos=ui->nameMusicList->currentRow();
    musicLists[pos].removeSqlMusic();          //先从数据库中删除歌曲信息
    //移除歌单
    int i=0;
    for(auto it=musicLists.begin();it!=musicLists.end(); )
    {
        if(i==pos)
        {
            it= musicLists.erase(it);
            break;
        }
        else
        {
            it++;
        }
        i++;
    }

    namelist_refresh();
}


//从我的歌单中移除歌曲
void MainWidget::musiclist_removeMusic()
{
    int pos = ui->myMusicList->currentRow();
    musicLists[musicLists_index].removeIndexMusic(pos);
    musicList_refresh();
}


//将歌曲从我的歌单中添加到我的喜欢
void MainWidget::musiclist_to_favor()
{
    int pos = ui->myMusicList->currentRow();
    ui->favorMusicWidget->musicList.addSingleMusic(ui->myMusicList->musicList.getIndexMusic(pos));
    ui->favorMusicWidget->refresh();
}


//将歌曲从我的歌单中添加当当前播放列表中
void MainWidget::musiclist_to_playlist()
{
    int pos = ui->myMusicList->currentRow();
    Music temp = ui->myMusicList->musicList.getIndexMusic(pos);
    ui->curMusicWidget->musicList.addSingleMusic(temp);
    ui->curMusicWidget->refresh();
    playlist->addMedia(temp.getUrl());       //WARNING:此处可能出现播放歌曲重复现象
}


//更换默认背景
void MainWidget::background_to_default()
{
    QSettings mysettings("./LightMusicPlayer.ini", QSettings::IniFormat);
    mysettings.setIniCodec("UTF8");
    QString  fileName = mysettings.value("defaultImage").toString();
    mysettings.setValue("backImageUrl1", fileName);          //将背景图片的历史修改
    setStyleSheet(QString("QWidget#Widget{"
                          "border-radius:10px;"
                          "border-image: url(%1);}").arg(fileName));
}


//切换自定义背景
void MainWidget::background_setting()
{
    QSettings mysettings("./LightMusicPlayer.ini", QSettings::IniFormat);
    mysettings.setIniCodec("UTF8");
    QString imagePath = mysettings.value("background/image-url").toString();
    if(imagePath.isEmpty())
    {
        //系统默认图片位置
        imagePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
    }

    //打开文件资源管理器
    QString fileName = QFileDialog::getOpenFileName(this,("选择自定义背景图片"),
                                                  imagePath,                                                  u8"图片文件(*jpg *png)");

    if(!fileName.isEmpty())
    {
        QImage testImage(fileName);
        if(!testImage.isNull())
        {
            QRegExp reg("([C|D|E].+/)");           //正则表达式匹配出文件的所在目录
            reg.indexIn(fileName);
            imagePath = reg.cap(0);
            //qDebug() << "imagepath: " << imagePath << endl;
            mysettings.setValue("background/image-url", imagePath);
            mysettings.setValue("backImageUrl1", fileName);
            setStyleSheet(QString("QWidget#Widget{"
                                  "border-radius:10px;"
                                  "border-image: url(%1);}").arg(fileName));
        }
    }
}




void MainWidget::on_btnPre_clicked()
{
    playlist->previous();           //直接返回播放列表中上一首，没有就直接反循环
}


//播放按钮
void MainWidget::on_btnPlay_clicked()
{
    if(player->state() == QMediaPlayer::PlayingState)
    {
        if(imageTimer->isActive())           //停止旋转图片
        {
            imageTimer->stop();
        }

        player->pause();
    }

    else if(player->state() == QMediaPlayer::PausedState)
    {
        if(!imageTimer->isActive())
        {
            imageTimer->start();
        }
        player->play();
    }

    //播放列表被添加音乐了，但是播放器没触发
    else if(!playlist->isEmpty() && (player->state() == QMediaPlayer::StoppedState))
    {
        playlist->setCurrentIndex(0);
        player->play();
        //qDebug() << "history" << historyPosition << endl;
        this->updatePosition(historyPosition);
//        player->setPosition(historyPosition);
//        ui->positionSlider->setValue(historyPosition);
    }
}


void MainWidget::on_btnPlayMode_clicked()
{
    //当前是列表循环
    if(playlist->playbackMode()==QMediaPlaylist::Loop)
    {
        ui->btnPlayMode->setStyleSheet(RandomStyle());
        ui->btnPlayMode->setToolTip(u8"随机播放");
        //系统托盘菜单活动修改图标文字
        action_systemTray_playmode->setIcon(QIcon(":/image/image/image/random2.png"));
        action_systemTray_playmode->setText(u8"随机播放");
        playlist->setPlaybackMode(QMediaPlaylist::Random);
    }

    else if(playlist->playbackMode()==QMediaPlaylist::Random)
    {
        ui->btnPlayMode->setStyleSheet(LoopOneStyle());
        ui->btnPlayMode->setToolTip(u8"单曲循环");
        action_systemTray_playmode->setIcon(QIcon(":/image/image/image/loop-one2.png"));
        action_systemTray_playmode->setText(u8"单曲循环");
        playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    }

    else if(playlist->playbackMode()==QMediaPlaylist::CurrentItemInLoop)
    {
        ui->btnPlayMode->setStyleSheet(LoopStyle());
        ui->btnPlayMode->setToolTip(u8"循环播放");
        action_systemTray_playmode->setIcon(QIcon(":/image/image/image/loop2.png"));
        action_systemTray_playmode->setText(u8"循环播放");
        playlist->setPlaybackMode(QMediaPlaylist::Loop);
    }
}


//从电脑中添加音乐到本地音乐歌单
void MainWidget::on_btnAddMusic_clicked()
{
    //从设置中读取音乐路径
    QSettings mysettings("./LightMusicPlayer.ini",QSettings::IniFormat);
    mysettings.setIniCodec("UTF8");
    QString fileName = mysettings.value("musicPath").toString();

    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    //setFileMode设置了添加时文件选中情况，ExistingFiles支持多选和ctrl+A
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setWindowTitle("添加本地音乐（注：自动过滤，按下\"Ctrl+A\"全选添加即可；不支持添加文件夹）");
    //设置mime类型文件过滤器，mime类型为描述文件或字节流格式和性质等的类型
    //关于通用的mime类型：https://www.cnblogs.com/xiaohi/p/6550133.html
    QStringList list;
    list<<"application/octet-stream";            //该格式类型会显示所有文件
    fileDialog.setMimeTypeFilters(list);
    //设置文件对话框的打开初始路径
    if(!fileName.isEmpty())
    {
        fileDialog.setDirectory(fileName);
    }

    else
    {
        fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first());
    }
    //fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first());
    //exec打开文件对话框，返回值等于Accepted表示打开成功
    if (fileDialog.exec() == QDialog::Accepted)
    {
       //selectedUrls返回所有被选中文件的url
       QList<QUrl> urls=fileDialog.selectedUrls();
       //qDebug() << urls << endl;
       //准备记载上次打开的路径
       if(!urls.isEmpty())
       {
           QString historyPath = urls[0].toLocalFile();
           QRegExp reg("([C|D|E].+/)");           //正则表达式匹配出文件的所在目录
           reg.indexIn(historyPath);
           historyPath = reg.cap(0);
           if(!historyPath.isEmpty())
           {
               mysettings.setValue("musicPath", historyPath);
           }
       }
       ui->localMusicWidget->musicList.addAllMusic(urls);
       ui->localMusicWidget->refresh();
       ui->stackedWidget->setCurrentIndex(3);       //切换到本地音乐页面
       stackPageIndex = 3;
    }
}


//打开或关闭音量滑动条
void MainWidget::on_btnVolume_clicked()
{
    if(ui->volumeSlider->isHidden())
    {
        ui->volumeSlider->show();
    }

    else
    {
        ui->volumeSlider->hide();
    }
}


//下一首
void MainWidget::on_btnNext_clicked()
{
    playlist->next();               //直接返回下一首
}


//当前播放音乐列表右键菜单
//对该控件右键会发送信号customContextMenuRequested，其中pos为鼠标所在坐标
void MainWidget::on_curMusicWidget_customContextMenuRequested(const QPoint &pos)
{
    //itemAt返回坐标pos处listWidget中的item，如果为空指针，则说明鼠标没有右键在歌曲上
    if(ui->curMusicWidget->itemAt(pos) == Q_NULLPTR)
    {
        return;
    }

    //执行此菜单，pos()为鼠标光点坐标，返回触发的QAction，此时action发送信号做出反应
    menu_currentPlay->exec(QCursor::pos());
}


//我的喜欢音乐右键菜单
void MainWidget::on_favorMusicWidget_customContextMenuRequested(const QPoint &pos)
{
    if(ui->favorMusicWidget->itemAt(pos) == Q_NULLPTR)
    {
        return;
    }

    menu_favorMusic->exec(QCursor::pos());
}


//本地音乐列表右键菜单
void MainWidget::on_localMusicWidget_customContextMenuRequested(const QPoint &pos)
{
    if(ui->localMusicWidget->itemAt(pos) == Q_NULLPTR)
    {
        return;
    }

    menu_localMusic->exec(QCursor::pos());
}



//我的歌单音乐列表右键菜单
void MainWidget::on_myMusicList_customContextMenuRequested(const QPoint &pos)
{
    if(ui->myMusicList->itemAt(pos) == Q_NULLPTR)
    {
        return;
    }

    menu_musicList->exec(QCursor::pos());
}


//搜索音乐列表右键菜单
void MainWidget::on_searchedMusicWidget_customContextMenuRequested(const QPoint &pos)
{
    if(ui->searchedMusicWidget->itemAt(pos) == Q_NULLPTR)
    {
        return;
    }

    menu_searchedMusic->exec(QCursor::pos());
}


//我的歌单名字列表右键菜单
void MainWidget::on_nameMusicList_customContextMenuRequested(const QPoint &pos)
{
    if(ui->nameMusicList->itemAt(pos) == Q_NULLPTR)
    {
        return;
    }

    menu_musicListName->exec(QCursor::pos());
}


//当前播放按钮点击切换界面
void MainWidget::on_btnCurMusic_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    stackPageIndex = 0;
}


//本地音乐按钮点击切换界面
void MainWidget::on_btnLocalMusic_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
    stackPageIndex = 3;
}


//我的喜欢按钮点击切换界面
void MainWidget::on_btnFavorMusic_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
    stackPageIndex = 2;
}


//切换到歌词页面
void MainWidget::on_btnLyric_clicked()
{
    if(ui->stackedWidget->currentIndex() != 5)
        ui->stackedWidget->setCurrentIndex(5);
    else
        ui->stackedWidget->setCurrentIndex(stackPageIndex);
}


//界面最小化
void MainWidget::on_btnMin_clicked()
{
    this->showMinimized();
}


//关闭程序按钮
void MainWidget::on_btnExit_clicked()
{
    //exit(0);
    //不重写closeEvent时close也可以退出程序；注意不能直接exit，运行close才能触发closeEvent事件
    close();
}


//切换皮肤按钮
void MainWidget::on_btnSkin_clicked()
{
    menu_skinSwitch->exec(QCursor::pos());          //激活菜单栏
}


//程序关于信息按钮
void MainWidget::on_btnAbout_clicked()
{
    QMessageBox::about(this, u8"关于"," LightMusicPlayer | 一款精致小巧的本地音乐播放器\n"
                                    " 作者：洲洲\n\n"
                                   "【歌词文件说明】与歌曲文件同名同目录（.lry文件）\n"
                                   "【音乐文件类型】可添加类型有（.mp3/.flac/.mpga文件）\n"
                                    " 添加音乐时可使用\ctrl+A\选择全部。\n"
                                    " 注：鼠标移动到不认识的按钮上，会有说明哦~\n");
}


//添加新的自定义歌单
void MainWidget::on_btnAddMusicList_clicked()
{
    bool ok;
    //QInputDialog提供一个小的输入对话框以获取输入
    //getText返回从小对话框中获取到的QString,如果用户点了取消ok为false,否则为true
    QString text=QInputDialog::getText(this, u8"新建歌单",u8"请输入新歌单的名字：", QLineEdit::Normal, "", &ok);
    if(ok && !text.isEmpty())
    {
        MusicList tempMusic;
        tempMusic.setName(text);
        musicLists.push_back(tempMusic);
        namelist_refresh();           //刷新并写入数据库
    }
}


//双击当前播放列表中的item
void MainWidget::on_curMusicWidget_doubleClicked(const QModelIndex &index)
{
    int i = index.row();          //获取元素位置
    playlist->setCurrentIndex(i);
    player->play();
}


//双击我的喜欢列表中的item
void MainWidget::on_favorMusicWidget_doubleClicked(const QModelIndex &index)
{
    //将我的喜欢列表设置到播放器中
    playlist->clear();
    ui->favorMusicWidget->musicList.addToPlayList(playlist);
    ui->curMusicWidget->setMusicList_Playing(ui->favorMusicWidget->musicList);
    int i = index.row();
    playlist->setCurrentIndex(i);
    player->play();
    ui->stackedWidget->setCurrentIndex(0);          //跳转到当前播放列表
    stackPageIndex = 0;
}


//双击本地音乐列表中的item
void MainWidget::on_localMusicWidget_doubleClicked(const QModelIndex &index)
{
    playlist->clear();
    ui->localMusicWidget->musicList.addToPlayList(playlist);
    ui->curMusicWidget->setMusicList_Playing(ui->localMusicWidget->musicList);
    int i = index.row();
    playlist->setCurrentIndex(i);
    player->play();
    ui->stackedWidget->setCurrentIndex(0);          //跳转到当前播放列表
    stackPageIndex = 0;
}


//双击搜索音乐列表的item
void MainWidget::on_searchedMusicWidget_doubleClicked(const QModelIndex &index)
{
    playlist->clear();
    ui->searchedMusicWidget->musicList.addToPlayList(playlist);
    ui->curMusicWidget->setMusicList_Playing(ui->searchedMusicWidget->musicList);
    int i = index.row();
    playlist->setCurrentIndex(i);
    player->play();
}


//双击我的歌单中的音乐item
void MainWidget::on_myMusicList_doubleClicked(const QModelIndex &index)
{
    playlist->clear();
    musicLists[musicLists_index].addToPlayList(playlist);
    ui->curMusicWidget->setMusicList_Playing(musicLists[musicLists_index]);
    int i = index.row();
    playlist->setCurrentIndex(i);
    player->play();
    ui->stackedWidget->setCurrentIndex(0);          //跳转到当前播放列表
    stackPageIndex = 0;
}


//双击自定义歌单名字列表
void MainWidget::on_nameMusicList_doubleClicked(const QModelIndex &index)
{
    ui->stackedWidget->setCurrentIndex(4);          //跳转到歌单内容列表
    stackPageIndex = 4;
    musicLists_index=index.row();
    musicList_refresh();                            //刷新歌单在播放列表中
}


//给我的歌单添加歌曲
void MainWidget::on_btnAddMyMusicList_clicked()
{
    MusicListWidgetDialog *dialog = new MusicListWidgetDialog();
    //准备从本地音乐歌单中添加歌曲给我的歌单
    int num = ui->localMusicWidget->count();
    bool *results = new bool[num];     //表示每首歌曲被选中情况
    //将本地歌单设置给对话框显示
    dialog->setMusicList(ui->localMusicWidget->musicList, results);
    //打开对话框点击确定后返回Accepted,否则返回拒绝
    if(dialog->exec() == QDialog::Accepted)
    {
        for(int i = 0; i < num; i++)
        {
            if(results[i])              //表示歌曲被选中
            {
                musicLists[musicLists_index].addSingleMusic(ui->localMusicWidget->musicList.getIndexMusic(i));
            }
        }

        musicList_refresh();        //刷新歌单
    }

    delete []results;
}


//给我的喜欢添加歌曲,从本地音乐中添加
void MainWidget::on_btnAddFavorMusic_clicked()
{
    MusicListWidgetDialog *dialog = new MusicListWidgetDialog(this);
    int num = ui->localMusicWidget->count();
    bool *results = new bool[num];
    dialog->setMusicList(ui->localMusicWidget->musicList, results);
    if(dialog->exec() == QDialog::Accepted)
    {
        //qDebug() << "start" << endl;
        for(int i=0; i < num; i++)
        {
            qDebug() << results[i] << endl;
        }

        for(int i=0; i < num; i++)
        {
            if(results[i])
            {
                ui->favorMusicWidget->musicList.addSingleMusic(ui->localMusicWidget->musicList.getIndexMusic(i));
                //qDebug() << "add music" << " ";
            }
        }
        //qDebug() << "favor" << ui->favorMusicWidget->musicList.musicList[0].getUrl() << endl;

        ui->favorMusicWidget->refresh();
    }

    else
    {
        qDebug() << "dialog exec error" << endl;
        QMessageBox::warning(this, "Dialog error", "open music file dialog error");
    }

    delete []results;
}


//标题按钮点击显示关于信息
void MainWidget::on_btnTitle_clicked()
{
    this->on_btnAbout_clicked();
}


//当前播放歌单清空
void MainWidget::on_btnClearCur_clicked()
{
    QMessageBox::StandardButton btn;
    btn = QMessageBox::question(this, "提示", "此操作不可逆！\nAre you sure???!!!", QMessageBox::Yes|QMessageBox::No);
    if (btn == QMessageBox::Yes)
    {
        ui->curMusicWidget->musicList.clear();
        ui->curMusicWidget->refresh();
        playlist->clear();              //清空播放器的歌单
    }
}


//清空我的喜欢
void MainWidget::on_btnFavorClear_clicked()
{
    QMessageBox::StandardButton btn;
    btn = QMessageBox::question(this, "提示", "此操作不可逆！\nAre you sure???!!!", QMessageBox::Yes|QMessageBox::No);
    if (btn == QMessageBox::Yes)
    {
        ui->favorMusicWidget->musicList.clear();
        ui->favorMusicWidget->refresh();
        //playlist->clear();              //清空播放器的歌单
    }
}


//整理我的喜欢
void MainWidget::on_btnFavorNeaten_clicked()
{
    ui->favorMusicWidget->musicList.neaten();
    ui->favorMusicWidget->refresh();
}


//根据歌名排序我的喜欢
void MainWidget::on_btnSortTitle_favor_clicked()
{
    ui->favorMusicWidget->musicList.sortMusic(SongTitle);
    ui->favorMusicWidget->refresh();
}


//根据歌手排序我的喜欢
void MainWidget::on_btnSortSinger_favor_clicked()
{
    ui->favorMusicWidget->musicList.sortMusic(Singer);
    ui->favorMusicWidget->refresh();
}


//根据时长排序我的喜欢
void MainWidget::on_btnSortDuration_favor_clicked()
{
    ui->favorMusicWidget->musicList.sortMusic(Duration);
    ui->favorMusicWidget->refresh();
}



//整理本地音乐
void MainWidget::on_btnLocalNeaten_clicked()
{
    ui->localMusicWidget->musicList.neaten();
    ui->localMusicWidget->refresh();
}


//清空本地音乐
void MainWidget::on_btnLocalClear_clicked()
{
    QMessageBox::StandardButton btn;
    btn = QMessageBox::question(this, "提示", "此操作不可逆！\nAre you sure???!!!", QMessageBox::Yes|QMessageBox::No);
    if (btn == QMessageBox::Yes)
    {
        ui->localMusicWidget->musicList.clear();
        ui->localMusicWidget->refresh();
    }
}


//根据歌手排序本地音乐
void MainWidget::on_btnSortSinger_local_clicked()
{
    //qDebug() << "sort local singer" << endl;
    ui->localMusicWidget->musicList.sortMusic(Singer);
    ui->localMusicWidget->refresh();
}


//根据时长排序本地音乐
void MainWidget::on_btnSortDuration_local_clicked()
{
    ui->localMusicWidget->musicList.sortMusic(Duration);
    ui->localMusicWidget->refresh();
}


//根据歌名排序本地音乐
void MainWidget::on_btnSortTitle_local_clicked()
{
    ui->localMusicWidget->musicList.sortMusic(SongTitle);
    ui->localMusicWidget->refresh();
}


//清空我的歌单
void MainWidget::on_btnMyMusicListClear_clicked()
{
    musicLists[musicLists_index].clear();
    musicList_refresh();
}


//整理我的歌单
void MainWidget::on_btnMyMusicListNeaten_clicked()
{
    musicLists[musicLists_index].neaten();
    musicList_refresh();
}


//根据时长排序我的歌单
void MainWidget::on_btnSortDuration_my_clicked()
{
    musicLists[musicLists_index].sortMusic(Duration);
    musicList_refresh();
}


//根据歌手排序我的歌单
void MainWidget::on_btnSortSinger_my_clicked()
{
    musicLists[musicLists_index].sortMusic(Singer);
    musicList_refresh();
}


//根据歌名排序我的歌单
void MainWidget::on_btnSortTitle_my_clicked()
{
    musicLists[musicLists_index].sortMusic(SongTitle);
    musicList_refresh();
}


//音量滑动条变化更新
void MainWidget::on_volumeSlider_valueChanged(int value)
{
    player->setVolume(value);
}


