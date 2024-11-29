#include <QApplication>
#include "musicplayer.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MusicPlayer player;
    player.resize(600, 400);
    player.show();

    return app.exec();
}
