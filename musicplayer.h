#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>

QT_BEGIN_NAMESPACE
namespace Ui { class MusicPlayer; }
QT_END_NAMESPACE

class MusicPlayer : public QWidget {
    Q_OBJECT

public:
    explicit MusicPlayer(QWidget *parent = nullptr);
    ~MusicPlayer();

private slots:
    void playMusic();
    void pauseMusic();
    void stopMusic();
    void addMusic();
    void playSelectedMusic();
    void updateProgress(qint64 position);
    void setDuration(qint64 duration);
    void seek(int position);
    void handleMediaStatus(QMediaPlayer::MediaStatus status);
    void handlePlaybackMode(); // 播放模式逻辑
    void addMusicFromFolder(); // 新功能：批量添加音乐文件
    void removeSelectedMusic(); // 新功能：移除选中的音乐
    void playSelectedOnDoubleClick(); // 双击播放选中的音乐
    void highlightPlayingMusic(bool clearAll = false); // 高亮或清除播放列表中的音乐
    void updatePlaylistWidget(); // 更新播放列表显示，仅显示音乐名


private:
    Ui::MusicPlayer *ui;
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    QStringList musicPaths; // 用于保存音乐文件路径
    QString currentPlaying; // 当前播放的音乐路径
};

#endif // MUSICPLAYER_H
