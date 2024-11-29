#include "musicplayer.h"
#include "ui_musicplayer.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QTime>
#include <QRandomGenerator>
#include <QDir>
#include <QFileDialog>

MusicPlayer::MusicPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MusicPlayer),
    player(new QMediaPlayer(this)),
    audioOutput(new QAudioOutput(this))
{
    ui->setupUi(this);

    // 设置播放器音频输出
    player->setAudioOutput(audioOutput);

    // 设置控件初始状态
    ui->progressBar->setRange(0, 0);

    // 连接信号与槽
    connect(ui->playButton, &QPushButton::clicked, this, &MusicPlayer::playMusic);
    connect(ui->pauseButton, &QPushButton::clicked, this, &MusicPlayer::pauseMusic);
    connect(ui->stopButton, &QPushButton::clicked, this, &MusicPlayer::stopMusic);
    connect(ui->addButton, &QPushButton::clicked, this, &MusicPlayer::addMusic);

    connect(player, &QMediaPlayer::positionChanged, this, &MusicPlayer::updateProgress);
    connect(player, &QMediaPlayer::durationChanged, this, &MusicPlayer::setDuration);
    connect(ui->progressBar, &QSlider::sliderMoved, this, &MusicPlayer::seek);

    connect(ui->playlistWidget, &QListWidget::itemDoubleClicked, this, &MusicPlayer::playSelectedMusic);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &MusicPlayer::handleMediaStatus);
    connect(ui->addFolderButton, &QPushButton::clicked, this, &MusicPlayer::addMusicFromFolder);
    connect(ui->removeButton, &QPushButton::clicked, this, &MusicPlayer::removeSelectedMusic);
    connect(ui->playlistWidget, &QListWidget::itemDoubleClicked, this, &MusicPlayer::playSelectedOnDoubleClick);


}

MusicPlayer::~MusicPlayer() {
    delete ui;
}

void MusicPlayer::playMusic() {
    if (ui->playlistWidget->currentRow() >= 0) {
        if (player->playbackState() == QMediaPlayer::StoppedState) {
            playSelectedMusic();
        } else {
            player->play();
        }
        ui->statusLabel->setText("状态: 播放中");
    }
}

void MusicPlayer::pauseMusic() {
    player->pause();
    ui->statusLabel->setText("状态: 暂停");
}

void MusicPlayer::stopMusic() {
    player->stop();
    ui->statusLabel->setText("状态: 停止");
    ui->progressBar->setValue(0);

    // 清除当前播放项
    currentPlaying.clear();

    // 清除所有高亮
    highlightPlayingMusic(true);
}

void MusicPlayer::addMusic() {
    QStringList files = QFileDialog::getOpenFileNames(this, "选择音乐文件", "", "音频文件 (*.mp3 *.wav *.ogg *.flac)");
    if (files.isEmpty()) {
        return;
    }

    for (const QString &file : files) {
        if (!musicPaths.contains(file)) {
            musicPaths.append(file);
        }
    }

    // 更新播放列表显示
    updatePlaylistWidget();

    ui->statusLabel->setText(QString("已添加 %1 首音乐").arg(files.size()));
}

void MusicPlayer::updatePlaylistWidget() {
    // 清空播放列表
    ui->playlistWidget->clear();

    // 填充播放列表，仅显示文件名
    for (const QString &path : musicPaths) {
        QFileInfo fileInfo(path);
        ui->playlistWidget->addItem(fileInfo.fileName());
    }

    // 恢复高亮
    highlightPlayingMusic();
}

void MusicPlayer::addMusicFromFolder() {
    // 弹出文件夹选择对话框
    QString folderPath = QFileDialog::getExistingDirectory(this, "选择文件夹", "");
    if (folderPath.isEmpty()) {
        return; // 用户未选择文件夹，直接返回
    }

    // 获取文件夹中的音乐文件
    QDir dir(folderPath);
    QStringList filters;
    filters << "*.mp3" << "*.wav" << "*.ogg" << "*.flac"; // 支持的文件格式
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);

    int addedCount = 0; // 记录新增音乐数量

    for (const QFileInfo &fileInfo : fileList) {
        QString filePath = fileInfo.absoluteFilePath();
        // 检查是否已存在于路径列表中，避免重复添加
        if (!musicPaths.contains(filePath)) {
            musicPaths.append(filePath); // 添加到路径列表
            ++addedCount; // 计数
        }
    }

    // 更新播放列表显示
    updatePlaylistWidget();

    // 更新状态栏提示
    if (addedCount > 0) {
        ui->statusLabel->setText(QString("已添加 %1 首音乐").arg(addedCount));
    } else {
        ui->statusLabel->setText("未添加任何音乐（可能是重复项）");
    }
}

void MusicPlayer::playSelectedMusic() {
    // 检查播放列表是否为空
    if (ui->playlistWidget->count() == 0) {
        ui->statusLabel->setText("播放列表为空，无法播放");
        return;
    }

    // 如果没有选中项，使用当前高亮项作为播放目标
    int currentIndex = ui->playlistWidget->currentRow();
    if (currentIndex < 0) {
        if (!currentPlaying.isEmpty()) {
            player->play(); // 如果已设置播放源但暂停，则直接播放
            ui->statusLabel->setText(QString("继续播放: %1").arg(QFileInfo(currentPlaying).fileName()));
        } else {
            ui->statusLabel->setText("未选择音乐，无法播放");
        }
        return;
    }

    // 设置当前播放项并更新播放源
    QString selectedMusic = musicPaths.at(currentIndex);
    if (currentPlaying != selectedMusic) {
        currentPlaying = selectedMusic; // 更新当前播放项
        player->setSource(QUrl::fromLocalFile(selectedMusic));
    }

    // 播放音乐
    player->play();
    ui->statusLabel->setText(QString("正在播放: %1").arg(QFileInfo(selectedMusic).fileName()));

    // 更新高亮
    highlightPlayingMusic();
}

void MusicPlayer::updateProgress(qint64 position) {
    ui->progressBar->setValue(position);
}

void MusicPlayer::setDuration(qint64 duration) {
    ui->progressBar->setRange(0, duration);
}

void MusicPlayer::seek(int position) {
    player->setPosition(position);
}

void MusicPlayer::handleMediaStatus(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::EndOfMedia) {
        handlePlaybackMode();
    }
}

void MusicPlayer::handlePlaybackMode() {
    int currentMode = ui->playModeBox->currentIndex();
    int currentIndex;
    for (int i = 0; i < ui->playlistWidget->count(); ++i) {
        QListWidgetItem *item = ui->playlistWidget->item(i);
        if (!currentPlaying.isEmpty() && musicPaths.at(i) == currentPlaying) {
            currentIndex = i;
        }
    }

    switch (currentMode) {
    case 0: // 顺序播放
        if (currentIndex < musicPaths.size() - 1) {
            ui->playlistWidget->setCurrentRow(currentIndex + 1);
            playSelectedOnDoubleClick();
        } else {
            stopMusic();
        }
        break;

    case 1: { // 随机播放
        int randomIndex = QRandomGenerator::global()->bounded(musicPaths.size());
        ui->playlistWidget->setCurrentRow(randomIndex);
        playSelectedOnDoubleClick();
        break;
    }

    case 2: // 循环播放
        if (currentIndex < musicPaths.size() - 1) {
            ui->playlistWidget->setCurrentRow(currentIndex + 1);
        } else {
            ui->playlistWidget->setCurrentRow(0);
        }
        playSelectedOnDoubleClick();
        break;
    }

    // 更新高亮
    highlightPlayingMusic();
}

void MusicPlayer::removeSelectedMusic() {
    int currentIndex = ui->playlistWidget->currentRow();
    if (currentIndex < 0) {
        ui->statusLabel->setText("未选择音乐，无法移除");
        return;
    }

    QString selectedMusic = musicPaths.at(currentIndex);

    // 如果正在播放该音乐，则停止播放并清除当前播放项
    if (currentPlaying == selectedMusic) {
        stopMusic();
    }

    // 从路径列表中移除
    musicPaths.removeAt(currentIndex);

    // 处理索引变化，确保顺序播放的正确性
    if (!currentPlaying.isEmpty()) {
        // 获取当前播放音乐的索引
        int playingIndex = musicPaths.indexOf(currentPlaying);

        // 如果当前播放音乐被移除，清除播放状态
        if (playingIndex == -1) {
            currentPlaying.clear();
        } else {
            // 更新播放列表的选中项以保持正确的顺序播放逻辑
            ui->playlistWidget->setCurrentRow(playingIndex);
        }
    }

    // 更新播放列表显示
    updatePlaylistWidget();

    // 更新状态栏提示
    ui->statusLabel->setText("音乐已移除");
}

void MusicPlayer::playSelectedOnDoubleClick() {
    int currentIndex = ui->playlistWidget->currentRow();
    if (currentIndex < 0) {
        return; // 未选择任何项
    }

    QString selectedMusic = musicPaths.at(currentIndex);

    // 如果正在播放该音乐，则不做任何操作
    if (player->source().toLocalFile() == selectedMusic &&
        player->playbackState() == QMediaPlayer::PlayingState) {
        return;
    }

    // 设置当前播放项
    currentPlaying = selectedMusic;
    player->setSource(QUrl::fromLocalFile(selectedMusic));
    player->play();

    // 更新高亮
    highlightPlayingMusic();

    // 更新状态栏
    ui->statusLabel->setText(QString("正在播放: %1").arg(QFileInfo(selectedMusic).fileName()));
}

void MusicPlayer::highlightPlayingMusic(bool clearAll) {
    for (int i = 0; i < ui->playlistWidget->count(); ++i) {
        QListWidgetItem *item = ui->playlistWidget->item(i);

        if (clearAll) {
            item->setBackground(Qt::NoBrush);
            item->setForeground(Qt::black);
        } else {
            item->setBackground(Qt::NoBrush);
            item->setForeground(Qt::black);

            // 如果当前播放的音乐路径与列表项匹配，则高亮
            if (!currentPlaying.isEmpty() && musicPaths.at(i) == currentPlaying) {
                item->setBackground(Qt::yellow);
                item->setForeground(Qt::blue);
            }
        }
    }
}
