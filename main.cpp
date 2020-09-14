#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    qSetMessagePattern("%{time process} %{if-debug}D%{endif}%{if-info}INFO%{endif}%{if-warning}WARNING%{endif}%{if-critical}CRITICAL%{endif}%{if-fatal}FATAL%{endif}%{if-category} %{category}%{endif}%{if-debug} %{file}:%{line}%{endif} - %{message}%{if-fatal} from %{backtrace [depth=3]}%{endif}");
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription(
                "VideoSwitch. Show videos and show word cloud in between."
            );
    parser.addOptions({
                          {{"W", "width"}, "screen width in pixels", "int"},
                          {{"H", "height"}, "screen height in pixels", "int"},
                          {"playlist", "playist json file path", "file"},
                          {"mask", "mask image file path", "file"},
                          {"wordlist", "word list file path", "file"},
                          {"image", "word cloud image file path", "file"},
                          {"program", "word cloud generator script file path", "file"},
                          {"regex", "regular expression for filtering words", "string"},
                          {"max_weight", "maximum weight of word added to word list", "int"},
                          {"default_weight", "default weight of word added to word list", "int"},
                          {"fade_in_duration", "duration of video fading in, in ms", "int"},
                          {"fade_out_duration", "duration of video fading out, in ms", "int"},
                          {"image_change_duration", "duration of image change transition, in ms", "int"},
                          {"font_size", "font size in control window", "int"},
                  });
    parser.addHelpOption();
    parser.process(app);
    MainWindow window;
    window.initParameters(parser);
    window.addVideoControls();
    window.show();
    return app.exec();
}
