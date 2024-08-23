#ifndef NEOLRCEDITORAPP_PLAYBACKCONTROLLER_H
#define NEOLRCEDITORAPP_PLAYBACKCONTROLLER_H

#include <memory>

#include <QObject>

namespace talcs {
    class OutputContext;
    class AudioSourcePlayback;
    class TransportAudioSource;
    class AudioFormatInputSource;
    class WaveformPainter;
}

class QFile;

class PlaybackController : public QObject {
    Q_OBJECT
public:
    explicit PlaybackController(QObject *parent = nullptr);
    ~PlaybackController() override;

    static PlaybackController *instance();

    bool initialize();

    bool openAudioFile(const QString &fileName);
    void closeAudioFile();
    QString audioFileName() const;

    int audioLengthTime() const;

    void setPlaying(bool isPlaying);
    bool isPlaying() const;

    void setPositionTime(int time);
    int positionTime() const;

    talcs::WaveformPainter *waveformPainter() const;

signals:
    void audioFileNameChanged(const QString &fileName);
    void playingChanged(bool isPlaying);
    int positionTimeChanged(int positionTime);

private:
    std::unique_ptr<QFile> m_audioFile;
    std::unique_ptr<talcs::AudioFormatInputSource> m_audioFormatInputSource;
    std::unique_ptr<talcs::TransportAudioSource> m_transportAudioSource;
    std::unique_ptr<talcs::AudioSourcePlayback> m_playback;
    std::unique_ptr<talcs::OutputContext> m_outputContext;

    std::unique_ptr<talcs::WaveformPainter> m_waveformPainter;
    std::unique_ptr<QFile> m_replicaAudioFile;
    std::unique_ptr<talcs::AudioFormatInputSource> m_replicaAudioFormatInputSource;

    bool m_isPlaying = false;
    int m_positionTime = 0;

};


#endif //NEOLRCEDITORAPP_PLAYBACKCONTROLLER_H
