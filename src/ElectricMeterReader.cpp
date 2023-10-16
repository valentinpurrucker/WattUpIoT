#include "ElectricMeterReader.h"
#include <Esp.h>

#include "unit.h"
#include "Debug.h"

// PUBLIC:
const int ElectricMeterReader::MAX_TRANSMISSION_DURATION_MS = 400;
const int ElectricMeterReader::MAX_READ_TIME = 2000;

const byte ElectricMeterReader::SML_START_SEQ[] = {0x1B, 0x1B, 0x1B, 0x1B, 0x01, 0x01, 0x01, 0x01};
const byte ElectricMeterReader::SML_END_SEQ[] = {0x1B, 0x1B, 0x1B, 0x1B, 0x1A};

ElectricMeterReader::ElectricMeterReader(Scheduler &scheduler) : mScheduler(scheduler)
{
}

ElectricMeterReader::ElectricMeterReader(MqttPublisher *mqtt, Scheduler &scheduler) : mMqtt(mqtt), mScheduler(scheduler)
{
}

void ElectricMeterReader::setup()
{
    mBufferSize = 3840;
    mBuffer = std::unique_ptr<byte[]>(new byte[mBufferSize]);
    mEMeterSerial = std::unique_ptr<SoftwareSerial>(new SoftwareSerial(D6));
    mEMeterSerial->begin(9600, EspSoftwareSerial::SWSERIAL_8N1);
    pinMode(D6, INPUT);
    reset();
}

bool ElectricMeterReader::update()
{

    switch (mCurrentState)
    {
    case EnergyMeterState::Uninitialized:
        break;
    case EnergyMeterState::Idle:
        break;
    case EnergyMeterState::Waiting:
        readSmlData();
        break;
    case EnergyMeterState::Reading:
        break;
    case EnergyMeterState::DataRead:
        parseSmlData();
        break;
    }

    yield();
    return true;
}

void ElectricMeterReader::reset()
{
    mCurrentState = EnergyMeterState::Idle;
    mReadingState = ReadingState::ReadingStart;
    mDataReadCallbackCalled = false;
    mChecksumByteNumber = 3;
    mCurrentBufferPosition = 0;
    mLastReadingStartTs = -1;
}

void ElectricMeterReader::read()
{
    D_Println(F("Read start"));
    mCurrentState = Reading;
    mReadingState = ReadingStart;
    memset(mBuffer.get(), 0, mBufferSize);
    mChecksumByteNumber = 3;
    mCurrentBufferPosition = 0;
}

void ElectricMeterReader::wait()
{
    D_Println(F("Reset waiting"));
    reset();
    mDataReadCallbackCalled = false;
    mCurrentState = Waiting;
}

void ElectricMeterReader::sendDataIfAvailable()
{
    if (!mCurrentData.has_value())
    {
        D_Println(F("No data to send"));
        return;
    }
    D_Println("sending");

    char s[60] = {};

    char pE[12 + 1] = {};
    char cP[5 + 1] = {};
    char timeStr[11] = {};

    sprintf(pE, "%.1f", mCurrentData.value().positiveEnergy);
    sprintf(cP, "%.0f", mCurrentData.value().currentPower);
    sprintf(timeStr, "%d", mScheduler.getTimestamp());

    sprintf(s, "{\"time\":%s,\"energy\":{\"power\":%s,\"total\":%s}}", timeStr, cP, pE);

    D_Println(s);

    mMqtt->publish(STR(MQTT_TOPIC), s);
}

void ElectricMeterReader::printMeterState()
{
    D_Printf("State: %d\n", mCurrentState);
}
void ElectricMeterReader::printReadingState()
{
    D_Printf("Reading: %d\n", mReadingState);
}

// PRIVATE:

void ElectricMeterReader::resetReadingState()
{
    mLastReadingStartTs = -1;
    mCurrentBufferPosition = 0;
}

bool ElectricMeterReader::readSmlData()
{

#if (MODE_SERIAL == 0)
    if (!Serial.available())
    {
        return true;
    }
#else
    if (!mEMeterSerial->available())
    {
        return true;
    }
#endif

    switch (mReadingState)
    {
    case ReadingState::NotReady:
        return true;
    case ReadingState::ReadingStart:
        readStartSequence();
        break;
    case ReadingState::ReadingMessage:
        readMessage();
        break;
    case ReadingState::ReadingChecksum:
        readChecksum();
        break;
    case ReadingState::ReadingFinished:
        mCurrentState = EnergyMeterState::DataRead;
        mDataAvailableToSend = true;
        mDataParsed = false;
        break;
    case ReadingState::Timeout:
        D_Println(F("Timeout"));
        reset();
    }

    return true;
}

void ElectricMeterReader::readStartSequence()
{

#if (MODE_SERIAL == 0)
    byte b = Serial.read();
#else
    byte b = mEMeterSerial->read();
#endif
    if (b != -1)
    {
        mBuffer[mCurrentBufferPosition] = b;

        if (mBuffer[mCurrentBufferPosition] == SML_START_SEQ[mCurrentBufferPosition])
        {
            mCurrentBufferPosition++;
        }

        if (mCurrentBufferPosition == sizeof(SML_START_SEQ))
        {
            mReadingState = ReadingMessage;
            mLastReadingStartTs = millis();
            return;
        }
        return;
    }
    return;
}

void ElectricMeterReader::readMessage()
{
#if (MODE_SERIAL == 0)
    byte b = Serial.read();
#else
    byte b = mEMeterSerial->read();
#endif
    if (b != -1)
    {
        if ((mCurrentBufferPosition + 3) == mBufferSize)
        {
            return;
        }
        mBuffer[mCurrentBufferPosition++] = b;

        // Check for end sequence
        int endSequenceEndIndex = sizeof(SML_END_SEQ) - 1;
        for (int i = 0; i <= endSequenceEndIndex; i++)
        {
            if (SML_END_SEQ[endSequenceEndIndex - i] != mBuffer[mCurrentBufferPosition - (i + 1)])
            {
                break;
            }
            if (i == endSequenceEndIndex)
            {
                // D_Println(F("msg read"));
                mReadingState = ReadingChecksum;
                return;
            }
        }
    }
}

void ElectricMeterReader::readChecksum()
{
#if (MODE_SERIAL == 0)
    byte b = Serial.read();
#else
    byte b = mEMeterSerial->read();
#endif
    if (b != -1 && mChecksumByteNumber > 0)
    {
        mBuffer[mCurrentBufferPosition++] = b;
        mChecksumByteNumber--;
    }
    if (mChecksumByteNumber == 0)
    {
        mReadingState = ReadingFinished;
    }
}

bool ElectricMeterReader::checkTimeout()
{
    if (mLastReadingStartTs == -1)
    {
        return true;
    }

    if (millis() - mLastReadingStartTs >= MAX_TRANSMISSION_DURATION_MS + 50)
    {
        mCurrentState = Idle;
        return false;
    }
    return true;
}

void ElectricMeterReader::parseSmlData()
{
    D_Printf(PSTR("Buf pos: %d\n"), mCurrentBufferPosition);
    sml_file *file = sml_file_parse(mBuffer.get() + 8, mCurrentBufferPosition - 16);

    for (int i = 0; i < file->messages_len; i++)
    {
        sml_message *message = file->messages[i];
        if (*message->message_body->tag == SML_MESSAGE_GET_LIST_RESPONSE)
        {
            sml_list *entry;
            sml_get_list_response *body;
            body = (sml_get_list_response *)message->message_body->data;
            for (entry = body->val_list; entry != NULL; entry = entry->next)
            {
                if (!entry->value)
                { // do not crash on null value
                    continue;
                }

                ObisCode code{
                    .mediumCode = entry->obj_name->str[0],
                    .channel = entry->obj_name->str[1],
                    .physicalUnit = entry->obj_name->str[2],
                    .measurementType = entry->obj_name->str[3],
                    .tarrif = entry->obj_name->str[4],
                    .other = entry->obj_name->str[5]};

                std::optional<EnergyMeterObisCodeType> type = getEnergyMeterObisCodeTypeFromCode(std::move(code));

                if (entry->value->type == SML_TYPE_OCTET_STRING)
                {
                    // char *str;
                    // free(str);
                }
                else if (entry->value->type == SML_TYPE_BOOLEAN)
                {
                    /*
                    D_Printf("%d-%d:%d.%d.%d*%d#%s#\n",
                                  entry->obj_name->str[0], entry->obj_name->str[1],
                                  entry->obj_name->str[2], entry->obj_name->str[3],
                                  entry->obj_name->str[4], entry->obj_name->str[5],
                                  entry->value->data.boolean ? "true" : "false");*/
                }
                else if (((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_INTEGER) ||
                         ((entry->value->type & SML_TYPE_FIELD) == SML_TYPE_UNSIGNED))
                {
                    double value = sml_value_to_double(entry->value);
                    int scaler = (entry->scaler) ? *entry->scaler : 0;
                    int prec = -scaler;
                    if (prec < 0)
                        prec = 0;
                    value = value * pow(10, scaler);

                    D_Printf(PSTR("---%d-%d:%d.%d.%d*%d#%.*f#"),
                             entry->obj_name->str[0], entry->obj_name->str[1],
                             entry->obj_name->str[2], entry->obj_name->str[3],
                             entry->obj_name->str[4], entry->obj_name->str[5], prec, value);

                    const char *unit = NULL;
                    if (entry->unit && // do not crash on null (unit is optional)
                        (unit = dlms_get_unit((unsigned char)*entry->unit)) != NULL)
                    {
                        D_Printf(PSTR("%s"), unit);
                        D_Println();
                    }

                    updateEnergyData(mCurrentData, type.value(), value);
                }
            }
            D_Println("----------------------");
        }
    }
}

ObisCode ElectricMeterReader::getObisCodeFromType(EnergyMeterObisCodeType type)
{
    ObisCode code;
    code.mediumCode = 1;
    code.channel = 0;

    switch (type)
    {
    case MANUFACTURER:
        code.physicalUnit = 96;
        code.measurementType = 50;
        code.tarrif = 1;
        code.other = 1;
        break;
    case DEVICE_ID:
        code.physicalUnit = 96;
        code.measurementType = 1;
        code.tarrif = 0;
        code.other = 255;
        break;
    case ENERGY_POSITIVE:
        code.physicalUnit = 1;
        code.measurementType = 8;
        code.tarrif = 0;
        code.other = 255;
        break;
    case ENERGY_NEGATIV:
        code.physicalUnit = 2;
        code.measurementType = 8;
        code.tarrif = 0;
        code.other = 255;
        break;
    case CURRENT_ACTIVE_POWER:
        code.physicalUnit = 16;
        code.measurementType = 7;
        code.tarrif = 0;
        code.other = 255;
        break;
    }

    return code;
}

std::optional<ElectricMeterReader::EnergyMeterObisCodeType> ElectricMeterReader::getEnergyMeterObisCodeTypeFromCode(ObisCode &&code)
{
    // Ensure common fields are correct
    if (code.mediumCode != 1 || code.channel != 0)
    {
        // Handle error or unexpected code here if necessary
        // E.g., throw an exception, return a default value, etc.
        return std::nullopt;
    }

    if (code.physicalUnit == 96 && code.measurementType == 50 && code.tarrif == 1 && code.other == 1)
    {
        return std::make_optional(MANUFACTURER);
    }
    if (code.physicalUnit == 96 && code.measurementType == 1 && code.tarrif == 0)
    {
        return std::make_optional(DEVICE_ID);
    }
    if (code.physicalUnit == 1 && code.measurementType == 8 && code.tarrif == 0)
    {
        return std::make_optional(ENERGY_POSITIVE);
    }
    if (code.physicalUnit == 2 && code.measurementType == 8 && code.tarrif == 0)
    {
        return std::make_optional(ENERGY_NEGATIV);
    }
    if (code.physicalUnit == 16 && code.measurementType == 7 && code.tarrif == 0)
    {
        return std::make_optional(CURRENT_ACTIVE_POWER);
    }

    return std::nullopt;
}

const char *ElectricMeterReader::getTypeStringFromObisType(EnergyMeterObisCodeType type)
{
    switch (type)
    {
    case MANUFACTURER:
        return "MANUFACTURER";
    case DEVICE_ID:
        return "DEVICE_ID";
    case ENERGY_POSITIVE:
        return "ACTIVE_POWER_POSITIVE";
    case ENERGY_NEGATIV:
        return "ACTIVE_POWER_NEGATIV";
    case CURRENT_ACTIVE_POWER:
        return "CURRENT_ACTIVE_POWER";
    default:
        return "";
    }
}

void ElectricMeterReader::updateEnergyData(std::optional<EnergyData> &data, EnergyMeterObisCodeType type, double value)
{
    if (!mCurrentData.has_value())
    {
        mCurrentData = std::optional<EnergyData>(EnergyData{});
    }

    switch (type)
    {
    case ENERGY_POSITIVE:
        data.value().positiveEnergy = value;
        break;
    case CURRENT_ACTIVE_POWER:
        data.value().currentPower = value;
        break;
    }
}