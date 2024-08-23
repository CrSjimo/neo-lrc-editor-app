#include "PlaybackController.h"

#include <QFile>

#include <TalcsCore/TransportAudioSource.h>
#include <TalcsDevice/OutputContext.h>
#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioSourcePlayback.h>
#include <TalcsFormat/AudioFormatIO.h>
#include <TalcsFormat/AudioFormatInputSource.h>
#include <TalcsGui/WaveformPainter.h>

static PlaybackController *m_instance = nullptr;

PlaybackController::PlaybackController(QObject *parent) : QObject(parent) {
    m_instance = this;
    m_outputContext = std::make_unique<talcs::OutputContext>();
    m_transportAudioSource = std::make_unique<talcs::TransportAudioSource>();
    m_transportAudioSource->setLoopingRange(0, 0);
    m_playback = std::make_unique<talcs::AudioSourcePlayback>(m_transportAudioSource.get());

    m_waveformPainter = std::make_unique<talcs::WaveformPainter>(m_waveformPainter.get());

    connect(m_transportAudioSource.get(), &talcs::TransportAudioSource::positionAboutToChange, this, [=](int position) {
        auto time = static_cast<int>(std::round(position / m_transportAudioSource->sampleRate() * 100));
        if (m_positionTime != time) {
            m_positionTime = time;
            emit positionTimeChanged(time);
        }
    });
}

PlaybackController::~PlaybackController() {
    m_instance = nullptr;
}

PlaybackController *PlaybackController::instance() {
    return m_instance;
}

bool PlaybackController::initialize() {
    if (!m_outputContext->initialize())
        return false;
    if (!m_outputContext->device()->start(m_playback.get()))
        return false;
    return true;
}

bool PlaybackController::openAudioFile(const QString &fileName) {
    auto file = std::make_unique<QFile>(fileName);
    if (!file->open(QIODevice::ReadOnly))
        return false;
    auto io = std::make_unique<talcs::AudioFormatIO>(file.get());
    if (!io->open(talcs::AbstractAudioFormatIO::Read)) {
        return false;
    }
    auto replicaFile = std::make_unique<QFile>(fileName);
    if (!replicaFile->open(QIODevice::ReadOnly))
        return false;
    auto replicaIO = std::make_unique<talcs::AudioFormatIO>(replicaFile.get());
    if (!replicaIO->open(talcs::AbstractAudioFormatIO::Read)) {
        return false;
    }

    setPlaying(false);
    m_audioFormatInputSource = std::make_unique<talcs::AudioFormatInputSource>();
    m_audioFormatInputSource->setAudioFormatIo(io.release(), true);
    m_audioFile = std::move(file);
    m_transportAudioSource->setSource(m_audioFormatInputSource.get());
    m_transportAudioSource->setPosition(0);
    m_transportAudioSource->setLoopingRange(0, m_audioFormatInputSource->length());
    m_positionTime = 0;

    m_replicaAudioFormatInputSource = std::make_unique<talcs::AudioFormatInputSource>();
    m_replicaAudioFormatInputSource->setAudioFormatIo(replicaIO.get(), true);
    m_replicaAudioFile = std::move(replicaFile);
    m_replicaAudioFormatInputSource->open(1024, replicaIO->sampleRate());
    m_waveformPainter->setSource(m_replicaAudioFormatInputSource.get(), replicaIO->channelCount(), replicaIO->length(), true);
    m_waveformPainter->startLoad(0, replicaIO->length());

    Q_UNUSED(replicaIO.release())

    emit audioFileNameChanged(fileName);
    return true;
}

void PlaybackController::closeAudioFile() {
    setPlaying(false);
    m_transportAudioSource->setSource(nullptr);
    m_audioFormatInputSource.reset();
    m_audioFile.reset();
    m_transportAudioSource->setPosition(0);
    m_transportAudioSource->setLoopingRange(0, 0);
    m_positionTime = 0;

    m_waveformPainter->setSource(nullptr, 0, 0);
    m_replicaAudioFormatInputSource.reset();
    m_replicaAudioFile.reset();

    emit audioFileNameChanged({});
}

QString PlaybackController::audioFileName() const {
    return m_audioFile ? m_audioFile->fileName() : QString();
}

int PlaybackController::audioLengthTime() const {
    return m_audioFormatInputSource ? static_cast<int>(std::round(static_cast<double>(m_audioFormatInputSource->length()) / m_transportAudioSource->sampleRate() * 100.0)) : 0;
}

void PlaybackController::setPlaying(bool isPlaying) {
    if (isPlaying == m_isPlaying)
        return;
    m_isPlaying = isPlaying;
    if (isPlaying) {
        m_transportAudioSource->play();
    } else {
        m_transportAudioSource->pause();
    }
}

bool PlaybackController::isPlaying() const {
    return m_isPlaying;
}

void PlaybackController::setPositionTime(int time) {
    if (m_positionTime != time) {
        QSignalBlocker blocker(m_transportAudioSource.get());
        m_positionTime = time;
        m_transportAudioSource->setPosition(static_cast<qint64>(std::round(time * m_transportAudioSource->sampleRate() / 100.0)));
        emit positionTimeChanged(time);
    }
}

int PlaybackController::positionTime() const {
    return m_positionTime;
}

talcs::WaveformPainter *PlaybackController::waveformPainter() const {
    return m_waveformPainter.get();
}
