#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    view(new QGraphicsView()),
    scene(new QGraphicsScene(this)),
    videoitem(new QGraphicsVideoItem()),
    picitem_fg(new QQGraphicsPixmapItem()),
    picitem(new QQGraphicsPixmapItem()),
    player(new QMediaPlayer()),
    playlist(new QMediaPlaylist),
    video_timer(new QTimer()),
    anim_group(new QParallelAnimationGroup()),
    video_anim(new QPropertyAnimation(videoitem, "opacity")),
    audio_anim(new QPropertyAnimation(player, "volume")),
    pic_fade_anim(new QPropertyAnimation(picitem, "opacity")),
    pic_change_anim(new QPropertyAnimation(picitem_fg, "opacity")),
    process(new QProcess(this)),
    lineedit(new QLineEdit(this)),
    weightedit(new QLineEdit(this)),
    slider(new QSlider(Qt::Horizontal, this)),
    logerr(new QLabel(this))
{
    // Create the layout for main window.
    setLayout(new QVBoxLayout());
    setFont(QFont("Titillium", 50));

    // Some GUI elements
    {
        QWidget *edit_widget = new QWidget(this);
        QHBoxLayout *edit_layout = new QHBoxLayout(edit_widget);
        layout()->addWidget(edit_widget);
        weightedit->setText("10");
        connect(lineedit, &QLineEdit::returnPressed, this, &MainWindow::updateWordcloud);
        connect(weightedit, &QLineEdit::returnPressed, this, &MainWindow::updateWordcloud);
        edit_layout->addWidget(lineedit, 4);
        edit_layout->addWidget(weightedit, 1);
    }

    {
        QPushButton *button = new QPushButton("update", this);
        connect(button, &QPushButton::released, this, &MainWindow::updateWordcloud);
        layout()->addWidget(button);
    }

    {
        QPalette palette;
        palette.setColor(QPalette::Foreground, Qt::red);
        logerr->setPalette(palette);
        logerr->setFont(QFont("Titillium", 20));
        layout()->addWidget(logerr);
    }

    {
        QPushButton *button = new QPushButton("stop video", this);
        connect(button, &QPushButton::released, this, &MainWindow::fadeOut);
        layout()->addWidget(button);
    }

    // Configure appearence of output widget.
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setStyleSheet("border: 0px");
    view->setScene(scene);
    view->setBackgroundBrush(QBrush(Qt::black));

    // Prepare animation group.
    anim_group->addAnimation(video_anim);
    anim_group->addAnimation(audio_anim);
    anim_group->addAnimation(pic_fade_anim);
    connect(anim_group, &QParallelAnimationGroup::finished, this, &MainWindow::videoAnimEnded);

    QFileInfo file = QFileInfo(pixmap_path);
    last_modified = file.lastModified();


    // Prepare background image of word cloud.
    scene->addItem(picitem);
    picitem->show();
    scene->addItem(picitem_fg);

    // Configure video widget and media player.
    player->setPlaylist(playlist);
    playlist->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);
    player->setVideoOutput(videoitem);
    scene->addItem(videoitem);
    videoitem->hide();
    connect(slider, &QSlider::sliderMoved, this, &MainWindow::setVideoPosition);
    connect(player, &QMediaPlayer::durationChanged, slider, &QSlider::setMaximum);
    connect(player, &QMediaPlayer::positionChanged, slider, &QSlider::setValue);

    // Prepare video timer (which triggers the fade out).
    video_timer->setSingleShot(true);
    connect(video_timer, &QTimer::timeout, this, &MainWindow::fadeOut);

    connect(process, SIGNAL(finished(int)), this, SLOT(updatePixmap(int)));
}

MainWindow::~MainWindow()
{
    delete pic_fade_anim;
    delete pic_change_anim;
    delete video_anim;
    delete audio_anim;
    delete anim_group;
    delete video_timer;
    delete player;
    delete playlist;
    delete scene;
    delete view;
}


void MainWindow::play()
{
    // Configure animation.
    video_anim->setDuration(fade_in_duration);
    video_anim->setStartValue(0.);
    video_anim->setEndValue(1.);
    video_anim->setEasingCurve(QEasingCurve::OutQuad);
    audio_anim->setDuration(fade_in_duration);
    audio_anim->setStartValue(0);
    audio_anim->setEndValue(100);
    audio_anim->setEasingCurve(QEasingCurve::OutExpo);
    pic_fade_anim->setDuration(fade_in_duration);
    pic_fade_anim->setStartValue(1.);
    pic_fade_anim->setEndValue(0.);
    pic_fade_anim->setEasingCurve(QEasingCurve::InQuad);
    // Start animation.
    videoitem->setOpacity(0.);
    player->play();
    videoitem->show();
    anim_group->start();
}

void MainWindow::videoAnimEnded()
{
    if (video_anim->endValue().toReal() > 0.5)
    {
        // The animation was fading in.
        if (player->duration() == 0)
            return;
        if (player->duration() - player->position() <= fade_out_duration)
        {
            // Alreadt time to fade out.
            fadeOut();
        }
        else
        {
            // No need to fade out yet. Update video_timer now in order to fade out in time.
            video_timer->setInterval(player->duration() - player->position() - fade_out_duration);
            video_timer->start();
        }
    }
    else
    {
        // The animation was fading out.
        player->stop();
        videoitem->hide();
        slider->setValue(0);
        slider->setMaximum(1);
    }
}

void MainWindow::fadeOut()
{
    if (!videoitem->isVisible())
        return;
    // Prepare animation.
    video_anim->setDuration(fade_out_duration);
    video_anim->setStartValue(1.);
    video_anim->setEndValue(0.);
    video_anim->setEasingCurve(QEasingCurve::InQuad);
    audio_anim->setDuration(fade_out_duration);
    audio_anim->setStartValue(100);
    audio_anim->setEndValue(0);
    audio_anim->setEasingCurve(QEasingCurve::InExpo);
    pic_fade_anim->setDuration(fade_out_duration);
    pic_fade_anim->setStartValue(0.);
    pic_fade_anim->setEndValue(1.);
    pic_fade_anim->setEasingCurve(QEasingCurve::OutQuad);
    // Start animation.
    anim_group->start();
}

void MainWindow::updatePixmap(const int exitcode)
{
    qDebug() << "updating pixmap";
    if (exitcode != 0)
    {
        qDebug() << "Python returned exit code" << exitcode;
        return;
    }
    QFileInfo file = QFileInfo(pixmap_path);
    if (file.lastModified() != last_modified)
    {
        last_modified = file.lastModified();
        QPixmap pixmap;
        pixmap.load(pixmap_path);
        if (videoitem->isVisible() && videoitem->opacity() > 0)
        {
            picitem->setPixmap(pixmap);
        }
        else {
            // Animate the transition between new and old image.
            // picitem_fg contains the old image, which fades out while
            // picitem already contains the new image.
            picitem_fg->setPixmap(picitem->pixmap());
            picitem_fg->show();
            picitem->setPixmap(pixmap);
            pic_change_anim->setDuration(picture_change_duration);
            pic_change_anim->setEasingCurve(QEasingCurve::InOutQuad);
            pic_change_anim->setStartValue(1.);
            pic_change_anim->setEndValue(0.);
            pic_change_anim->start();
        }
    }
}

void MainWindow::loadJson(const QString json_path)
{
    QFile json_file(json_path);
    if (!json_file.exists())
    {
        qWarning() << "File not found:" << json_path;
        return;
    }
    json_file.open(QFile::ReadOnly);
    const QJsonDocument doc = QJsonDocument::fromJson(json_file.readAll());
    if (doc.isNull() || doc.isEmpty())
    {
        qWarning() << "File is empty or loading failed:" << json_path;
        return;
    }
    QJsonObject obj = doc.object();
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
    {
        playlist->addMedia(QUrl::fromLocalFile(it->toString()));
        QPushButton *button = new QPushButton(it.key(), this);
        video_buttons.append(button);
        connect(button, &QPushButton::released, this, &MainWindow::chooseVideo);
        layout()->addWidget(button);
    }
}

void MainWindow::chooseVideo()
{
    if (sender() == nullptr)
    {
        qWarning() << "chooseVideo() should not be called directly, but only by events from buttons.";
        return;
    }
    const int idx = video_buttons.indexOf(static_cast<QPushButton*>(sender()), 0);
    if (idx == -1)
    {
        qWarning() << "Could not find out which button was pushed.";
        return;
    }
    playlist->setCurrentIndex(idx);
    play();
}

void MainWindow::updateWordcloud()
{
    logerr->setText("");
    if (!lineedit->text().isEmpty())
    {
        if (word_regex.exactMatch(lineedit->text()))
        {
            QFile file(wordlist_path);
            file.open(QFile::Append);
            bool ok;
            int weight = weightedit->text().toUInt(&ok);
            if (!ok)
            {
                qWarning() << "Invalid weight" << weightedit->text();
                logerr->setText("<b>Ignored input:</b> Invalid weight " + weightedit->text() + " (must be a positive integer)");
                weightedit->setText(QString::number(defaultweight));
                return;
            }
            else if (weight > maxweight)
                weight = maxweight;
            const QByteArray rawtext = (lineedit->text() + " ").toUtf8();
            while (weight-- > 0)
                file.write(rawtext);
            file.write("\n");
            file.close();
            lineedit->clear();
            weightedit->setText(QString::number(defaultweight));
        }
        else
        {
            qWarning() << "Invalid word:" << lineedit->text();
            logerr->setText("<b>Ignored input:</b> Invalid word.");
            return;
        }
    }
    process->start();
}

void MainWindow::initParameters(const QCommandLineParser &parser)
{
    // Screen size.
    if (!parser.value("width").isEmpty())
    {
        const int width = parser.value("width").toUInt();
        if (width > 9 && width < 10000)
            window_size.setWidth(width);
        else
            qWarning() << "Invalid width given:" << parser.value("width");
    }
    if (!parser.value("height").isEmpty())
    {
        const int height = parser.value("height").toUInt();
        if (height > 9 && height < 10000)
            window_size.setHeight(height);
        else
            qWarning() << "Invalid height given:" << parser.value("height");
    }
    view->setGeometry(0, 0, window_size.width(), window_size.height());
    view->show();
    videoitem->setSize(window_size);

    // String valued arguments.
    if (!parser.value("regex").isEmpty())
        word_regex = QRegExp(parser.value("regex"));
    if (!parser.value("playlist").isEmpty())
        playlist_path = parser.value("playlist");
    loadJson(playlist_path);
    if (!parser.value("image").isEmpty())
        pixmap_path = parser.value("image");
    QPixmap pixmap;
    pixmap.load(pixmap_path);
    picitem->setPixmap(pixmap);

    // Arguments required for external word cloud program.
    if (!parser.value("program").isEmpty())
        program_path = parser.value("program");
    if (!parser.value("mask").isEmpty())
        mask_path = parser.value("mask");
    if (!parser.value("wordlist").isEmpty())
        wordlist_path = parser.value("wordlist");
    // Prepare external process for updating word cloud.
    process->setProgram(program_path);
    process->setArguments({
                              "input=" + wordlist_path,
                              "output=" + pixmap_path,
                              "mask=" + mask_path
                          });

    // Integer valued arguments.
    if (!parser.value("max_weight").isEmpty())
    {
        const int weight = parser.value("max_weight").toUInt();
        if (weight > 0)
            maxweight = weight;
        else
            qWarning() << "Invalid value for max_weight:" << parser.value("max_weight");
    }
    if (!parser.value("default_weight").isEmpty())
    {
        const int weight = parser.value("default_weight").toUInt();
        if (weight > 0)
            defaultweight = weight;
        else
            qWarning() << "Invalid value for default_weight:" << parser.value("default_weight");
    }
    if (!parser.value("fade_in_duration").isEmpty())
    {
        bool ok;
        const int duration = parser.value("fade_in_duration").toUInt(&ok);
        if (ok)
            fade_in_duration = duration;
        else
            qWarning() << "Invalid value for fade_in_duration:" << parser.value("fade_in_duration");
    }
    if (!parser.value("fade_out_duration").isEmpty())
    {
        bool ok;
        const int duration = parser.value("fade_out_duration").toUInt(&ok);
        if (ok)
            fade_out_duration = duration;
        else
            qWarning() << "Invalid value for fade_out_duration:" << parser.value("fade_out_duration");
    }
    if (!parser.value("image_change_duration").isEmpty())
    {
        bool ok;
        const int duration = parser.value("image_change_duration").toUInt(&ok);
        if (ok)
             picture_change_duration = duration;
        else
            qWarning() << "Invalid value for image_change_duration:" << parser.value("image_change_duration");
    }
    if (!parser.value("font_size").isEmpty())
    {
        const int fontsize = parser.value("font_size").toUInt();
        if (fontsize > 3)
        {
            setFont(QFont("Titillium", fontsize));
            logerr->setFont(QFont("Titillium", fontsize/2));
        }
        else
            qWarning() << "Invalid value for font_size:" << parser.value("font_size");
    }
}

void MainWindow::playPauseVideo()
{
    if (!videoitem->isVisible() || videoitem->opacity() < 0.1)
        return;
    if (player->state() == QMediaPlayer::PlayingState)
        player->pause();
    else if (player->state() == QMediaPlayer::PausedState)
    {
        player->play();
        video_timer->start(player->duration() - player->position() - fade_out_duration);
    }
}

void MainWindow::addVideoControls()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *hlayout = new QHBoxLayout(widget);
    QIcon icon = QIcon::fromTheme("media-playback-pause", QIcon("./pause.svg"));
    QPushButton *playbutton = new QPushButton(icon, "", widget);
    connect(playbutton, &QPushButton::released, this, &MainWindow::playPauseVideo);
    hlayout->addWidget(playbutton, 0);
    hlayout->addWidget(slider, 1);
    layout()->addWidget(widget);
}

void MainWindow::setVideoPosition(const qint64 pos)
{
    player->setPosition(pos);
    if (player->state() == QMediaPlayer::PlayingState)
    {
        if (player->duration() - pos > fade_out_duration)
            video_timer->start(player->duration() - pos - fade_out_duration);
        else
            fadeOut();
    }
}
