#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <QDebug>
#include <QPushButton>
#include <QDateTime>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QCommandLineParser>
#include <QSlider>


/// Copy of QGraphicsPixmapItem, which can be used in the Qt animation framework.
class QQGraphicsPixmapItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
};

/// Main window contains only a few buttons and a QLineEdit, but also owns the
/// second window showing the picture / video output.
class MainWindow : public QWidget
{
    Q_OBJECT

    /// Standalone widget, which should be shown on large screen.
    QGraphicsView *view;
    /// Graphics scene for view.
    QGraphicsScene *scene;
    /// Video shown on large screen.
    QGraphicsVideoItem *videoitem;
    /// Auxilliary pixmap item, only required for transition when the image changes.
    QQGraphicsPixmapItem *picitem_fg;
    /// Image of the word cloud.
    QQGraphicsPixmapItem *picitem;
    /// Media player for videoitem.
    QMediaPlayer *player;
    /// Media playlist for videoitem.
    QMediaPlaylist *playlist;
    /// Timer to fade out the video.
    QTimer* video_timer;
    /// Animation group for fading in or out the video.
    QParallelAnimationGroup *anim_group;
    /// Fade in or out the video.
    QPropertyAnimation *video_anim;
    /// Fade in or out the audio volume.
    QPropertyAnimation *audio_anim;
    /// Fade out / in the word cloud when a video starts / ends.
    QPropertyAnimation *pic_fade_anim;
    /// Fade out picitem_fg when word cloud image changes.
    QPropertyAnimation *pic_change_anim;
    /// List of push buttons for video selection.
    QList<QPushButton*> video_buttons;
    /// External process for updating the word cloud png image.
    QProcess *process;
    /// Line edit to add words to the word list for the word cloud.
    QLineEdit *lineedit;
    /// Line edit to change the weight of a word added to the word cloud.
    QLineEdit *weightedit;
    /// Slider showing position in video.
    QSlider *slider;

    /// Time stamp of last modification of the word cloud image.
    QDateTime last_modified;

    QString playlist_path = "playlist.json";
    QString program_path = "gen_wordcloud.py";
    QString pixmap_path = "/tmp/wordcloud.png";
    QString mask_path = "mask.png";
    QString wordlist_path = "/tmp/wordlist.txt";

    /// Video fade in durations in ms.
    int fade_in_duration = 1000;
    /// Video fade out durations in ms.
    int fade_out_duration = 1000;
    /// Pixmap change durations in ms.
    int picture_change_duration = 500;
    /// Screen size in pixels.
    QSize window_size = {1920, 1080};
    /// Maximum weight of word added to word cloud.
    int maxweight = 100;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    /// Load videos for playlist.
    void loadJson(const QString json_path);
    /// Fade in and play video.
    void play();
    /// Initialize parameters from command line options.
    void initParameters(const QCommandLineParser &parser);
    void addVideoControls();

public slots:
    /// Some things need to be done when the video animation finishes.
    void videoAnimEnded();
    /// Fade out video.
    void fadeOut();
    /// Add word to the word list (for generating the word cloud) from lineedit
    /// and update the word cloud using the external process. When the process
    /// has finished, updatePixmap is called.
    void updateWordcloud();
    /// Update the word cloud image (in a smooth animation).
    void updatePixmap(const int exitcode);
    /// Select a video, should only be called from QPushButton events.
    void chooseVideo();
    /// Play or pause video if a video is currently visible.
    void playPauseVideo();
    /// Set video posiion and update video_timer.
    void setVideoPosition(const qint64 pos);
};
#endif // MAINWINDOW_H
