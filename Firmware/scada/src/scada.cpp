#include "scada.h"

using namespace scada;

Entry& Entry::instance()
{
    static Entry s_entry;
    return s_entry;
}

void IRAM_ATTR Entry::OnInputReadyISR(void *arg)
{
    reinterpret_cast<Entry *>(arg)->m_inputReady = true;
}

void IRAM_ATTR Entry::OnRPMPulseISR(void *arg)
{
    reinterpret_cast<Entry *>(arg)->m_currentTime = micros();
}

Entry::Entry()
    : m_wifiManager()
    , m_server(80)
    , m_serverInitialized(false)
    , m_asyncWorker(nullptr)
    , m_hostState(HostState::initializing)
    , m_waitTimer()
#if USE_ADS1115
    , m_ads()
#elif USE_ADS1220
    , m_ads()
#endif

#if USE_MCP4725
    , m_dac()
#endif
    , m_lcd(0x3F, 20, 4)
    , m_inputReady(false)
    , m_lastTime(0)
    , m_currentTime(0)
{

}

Entry::~Entry()
{

}

void Entry::setup()
{
    Serial.begin(115200);
    SPIFFS.begin();

    m_lcd.begin();
    m_lcd.backlight();

    m_lcd.setCursor(0, 0);
    m_lcd.print("Starting...");

    delay(500);
    yield();

#if USE_ADS1115
    m_ads.begin();
    m_ads.setDataRate(RATE_ADS1115_860SPS);
    m_ads.setGain(GAIN_EIGHT);
#elif USE_ADS1220
    m_ads.begin(CONSTANTS::SPI_CS_PIN, CONSTANTS::IN_READY_PIN);
    m_ads.set_conv_mode_single_shot();
    m_ads.set_data_rate(DR_1000SPS);
    m_ads.set_pga_gain(PGA_GAIN_1);
#endif

#if USE_MCP4725
    m_dac.begin(0x60);
#endif

    //pinMode(CONSTANTS::OUT_SWON_PIN, OUTPUT);
    //pinMode(CONSTANTS::OUT_SWFR_PIN, OUTPUT);
    //pinMode(CONSTANTS::IN_READY_PIN, INPUT);

#if USE_ADS1115 || USE_ADS1220
    attachInterruptArg(digitalPinToInterrupt(IN_READY_PIN), &Entry::OnInputReadyISR, this, FALLING);
#endif

    attachInterruptArg(digitalPinToInterrupt(IN_READY_PIN), &Entry::OnRPMPulseISR, this, FALLING);

    m_lastTime = micros();
    m_currentTime = micros();

    initAsyncWorker();
    m_stepTimer.framesPerSecond(100);
    m_stepTimer.fixedTimeStep(true);
    m_stepTimer.reset();

    m_lcd.setCursor(0, 1);
    m_lcd.print("State: Stopped");

    postTask([=]()
    {
        //m_lcd.setCursor(0,1);
        //m_lcd.print("State: Calibrating");
        //CalibrateSensors();
        //m_lcd.setCursor(0,1);
        //m_lcd.print("State: Stopped    "); 
    });
}

void Entry::loop()
{
    dispatchAll();
}

void Entry::initAsyncWorker()
{
    xTaskCreatePinnedToCore(
        [](void *arg){ reinterpret_cast<Entry *>(arg)->onAsyncWorker(); },
        "App::OnAsyncWorker", 20480, this, 1, &m_asyncWorker, 0);
}

void Entry::onAsyncWorker()
{
    delay(1000);
    yield();
    m_waitTimer.fixedTimeStep(true);
    m_waitTimer.framesPerSecond(100);
    m_waitTimer.reset();
    while (true)
    {
        updateServer();
        delay(1);
        yield();
    }
}

void Entry::postTask(std::function<void()> task)
{
    Locker<Mutex> locker = m_mutex;
    m_queue.push(task);
}

void Entry::dispatchAll()
{
    std::function<void()> method;
    m_mutex.Lock();
    while (m_queue.size() > 0)
    {
        method = std::move(m_queue.front());
        m_queue.pop();
        m_mutex.Unlock();
        try
        {
            method();
        }
        catch (...)
        {
        }
        m_mutex.Lock();
    }
    m_mutex.Unlock();
}