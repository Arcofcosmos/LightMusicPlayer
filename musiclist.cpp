#include "musiclist.h"
#include <QProgressDialog>
#include <QMimeDatabase>
#include <QtSql>
#include <QDesktopServices>
#include "musiclistwidget.h"


MusicList::MusicList(const QList<QUrl> &urlList, QString name)
{
    addAllMusic(urlList);
    setName(name);
}


void MusicList::addAllMusic(const QList<QUrl> urlList)
{
    //为添加歌曲增加进度对话框
    QProgressDialog progressDialog(u8"添加进度", u8"取消", 0, urlList.size());
    progressDialog.setMinimumSize(350,150);
    progressDialog.setWindowTitle("添加中...请稍后");
    progressDialog.show();

    int x = 0;
    for(auto i : urlList)
    {
        QMimeDatabase mimeDb;              //mime类型表示文档或者数据等的性质和格式
        QMimeType mime = mimeDb.mimeTypeForFile(i.toLocalFile());        //从歌曲文件中获取其Mime值
        //qDebug() << i.toLocalFile();

        if(mime.name()!="audio/mpeg" && mime.name()!="audio/flac" && mime.name() != "audio/ncm")        //过滤其它格式文件
        {
            continue;
        }


        musicList.push_back(Music(i));

        x++;
        progressDialog.setValue(x);
        //qDebug() << "music info: " << i << endl;
        //若需要则添加歌单到数据库中
        if(sql_flag)
        {
            musicList[musicList.size() - 1].insertDatabase(musicListName);
        }

        if(progressDialog.wasCanceled())      //进度被取消则退出
        {
            break;
        }
    }
}


//添加单首音乐
void MusicList::addSingleMusic(const Music &music)
{
    musicList.push_back(music);
    if(sql_flag)
    {
        musicList[musicList.size() - 1].insertDatabase(musicListName);
    }
}


//获取指定索引的歌曲
Music MusicList::getIndexMusic(int pos)
{
    return musicList[pos];
}


//移除指定位置的歌曲
void MusicList::removeIndexMusic(int pos)
{
    if(sql_flag)
    {
        removeSqlMusic();
        int i = 0;
        for(auto it = musicList.begin(); it != musicList.end();)
        {
            if(pos == i)
            {
                it = musicList.erase(it);
                break;
            }

            else
            {
                ++it;
            }
            i++;
        }

        insertMusicToSql();
    }

    else
    {
        int i = 0;
        for(auto it = musicList.begin(); it != musicList.end();)
        {
            if(pos == i)
            {
                it = musicList.erase(it);
                break;
            }

            else
            {
                ++it;
            }
            i++;
        }
    }
}


//移除数据库中的歌曲
void MusicList::removeSqlMusic()
{
    QSqlQuery sqlQueue;
    QString sqlCommand("delete from MusicInfo where name = ?");
    sqlQueue.prepare(sqlCommand);
    sqlQueue.addBindValue(musicListName);
    sqlQueue.exec();
}


//将歌单中歌曲插入到数据库中
void MusicList::insertMusicToSql()
{
    for(auto it : musicList)
    {
        it.insertDatabase(musicListName);
    }
}


//打开指定位置歌曲所在的文件夹
void MusicList::openFolderOfIndexMusic(int pos)
{
    QString s = musicList[pos].getUrl().toString();      //获取歌曲路径
    s.remove(s.split("/").last());       //移除歌曲名获取歌曲所在目录
    QDesktopServices::openUrl(s);        //打开指定目录，注目录应使用"/",使用"\"会打不开
}


//从数据库中读取歌曲到歌单
void MusicList::readSqlMusic()
{
    QSqlQuery sql_query;
    QString select_sql = "select url, singer, songTitle, duration, albumTitle, audioBitRate from MusicInfo where name = ?";
    sql_query.prepare(select_sql);
    sql_query.addBindValue(musicListName);
    if(sql_query.exec())         //指令执行成功
    {
        while(sql_query.next())      //一条条获取查询结果
        {
            Music tempMusic;
            tempMusic.url=QUrl(sql_query.value(0).toString());
            tempMusic.singer=sql_query.value(1).toString();
            tempMusic.songTitle=sql_query.value(2).toString();
            tempMusic.duration=sql_query.value(3).toInt();
            tempMusic.albumTitle=sql_query.value(4).toString();
            tempMusic.audioBitRate=sql_query.value(5).toInt();
            musicList.push_back(tempMusic);
        }
    }
}


//对歌单中歌曲进行排序
void MusicList::sortMusic(CompareMode key)
{
    sort(musicList.begin(), musicList.end(), MusicSortMode(key));
    //NOTE:是否需要对数据库进行更新还不得而知
}


//清空歌单
void MusicList::clear()
{
    musicList.clear();
    if(sql_flag)
    {
        removeSqlMusic();
    }
}


//将歌单中歌曲添加到 播放列表中
void MusicList::addToPlayList(QMediaPlaylist *playList)
{
    for(auto temp : musicList)
    {
        playList->addMedia(temp.getUrl());
    }
}


//将歌单放入listWidget中
void MusicList::addToListWidget(MusicListWidget *listWidget)
{
    for(auto temp : musicList)
    {
        QListWidgetItem *item = new QListWidgetItem();
        item->setIcon(listWidget->getIcon());
        item->setText(temp.getMusicInfo());
        listWidget->addItem(item);
        //qDebug() << "listWidget count: " << listWidget->count() << endl;
    }
}


//显示歌曲详细信息
void MusicList::showMusicDetailInfo(int pos)
{
    musicList[pos].showMusicDetailInfo();
}


//去除歌单中重复歌曲并排序
void MusicList::neaten()
{
    sort(musicList.begin(), musicList.end(), MusicSortMode(Default));     //unique删除相邻的一个重复单位，所以先排序
    //unique返回首个重复单位的地址，erase删除从该地址到end的所有单位，因为重复单位一般都是在后面添加进来的
    vector<Music>::iterator p;
    musicList.erase(unique(musicList.begin(), musicList.end(), MusicSortMode(Equality)), musicList.end());

    if(sql_flag)
    {
        removeSqlMusic();
        insertMusicToSql();
    }
}



