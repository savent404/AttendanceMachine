#include "opera.hpp"
#define OP_Finger 0
#define OP_RFID 1
#define OP_Room 2
#define OP_Manager 3
#define OP_Unknow 4

STP_ServerRS485* server;
STP_KeyMat* keyboard;
STP_RTC* rtc;

static bool normal_op(Opera* op, const char*& outputErrorString)
{
	if (op->init() == false) {
		outputErrorString = op->getErrorString();
		return false;
	}
	do {
		if (op->loop() == false) {
			if (op->getResCode() == Opera::USR_Cancel) {
				continue;
			} else if (op->getResCode() == Opera::USR_Error) {
				outputErrorString = op->getErrorString();
				op->deinit();
				return false;
			} else if (op->getResCode() == Opera::OK) {
				break;
			} else {
				outputErrorString = op->getErrorString();
				op->deinit();
				return false;
			}
		}
	} while (0);
	op->deinit();
	return true;
}
static bool submode_op(uint8_t RunningMode, uint8_t submode)
{
	Opera* get_op = NULL;
	if (RunningMode == OP_Finger && submode == 0) {
		get_op = new Opera_getFinger(*keyboard, true);
	} else if (RunningMode == OP_Finger && submode == 1) {
		get_op = new Opera_getFinger(*keyboard, false);
	} else if (RunningMode == OP_RFID) {
		get_op = new Opera_getNFC(*keyboard);
	} else if (RunningMode == OP_Room) {
		get_op = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getPassword);
	} else if (RunningMode == OP_Manager) {
		// TODO: Add manager code
	}
	if (submode == 0) {
		do {
			Opera* get_room = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getRoomID);
			// TODO: UI:输入房间号
			get_room->init();
			get_room->loop();
			if (get_room->getResCode() == Opera::USR_Cancel) {
				get_room->deinit();
				delete get_room;
			} else {
				// TODO: UI:显示错误信息
				const char* str = get_room->getErrorString();
				get_room->deinit();
				delete get_room;
				continue;
			}
			// TODO: UI:显示当前输入模式
			if (get_op->init() == false) {
				const char* str = get_op->getErrorString();
				// TODO: UI:显示错误信息
				get_op->deinit();
				continue;
			}
			if (get_op->loop() == false) {
				if (get_op->getResCode() == Opera::USR_Cancel) {
					get_op->deinit();
					continue;
				} else if (get_op->getResCode() == Opera::OK) {

				} else {
					const char* str = get_op->getErrorString();
					// TODO: UI:显示错误信息
					get_op->deinit();
					continue;
				}
			}
			get_op->deinit();
			// TODO: UI:操作中
			std::list<DB_Usr>::iterator node;
			DB_Sheet* sheet;
			for (int i = 1; i <= 3; i++) {
				DB_Base* base;
				switch (RunningMode) {
				case OP_Finger:
					base = new DB_FingerPrint(get_op->getAns());
					break;
				case OP_RFID:
					base = new DB_RFID(get_op->getAns());
					break;
				case OP_Room:
					base = new DB_Password(get_op->getAns());
					break;
				default:
					base = NULL;
				}
				sheet = new DB_Sheet((enum DB_Sheet::DB_Sector)i);
				node = sheet->search(*base);
				if (node != NULL)
					break;
				delete sheet;
			}
			if (node == NULL) {
				// TODO: UI:not match
				continue;
			}
			delete sheet;
			// 发送ROOMID 到下位机
			char buffer[4];
			const uint8_t* data = node->rid.data();
			sprintf(buffer, "%02d%02d", *data, *(data + 1));
			server->sendMessage((uint8_t*)buffer, 4);
			server->reciMessage();
			// TODO: UI:操作成功
		} while (1);
	} else {
		do {
			uint8_t room[2] = { 0xFF, 0xFF };
			Opera* get_room = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getRoomID);
			// TODO: UI:输入房间号
			get_room->init();
			get_room->loop();
			if (get_room->getResCode() == Opera::USR_Cancel) {
				memcpy(room, get_room->getAns(), 2);
				get_room->deinit();
				delete get_room;
			} else {
				// TODO: UI:显示错误信息
				const char* str = get_room->getErrorString();
				get_room->deinit();
				delete get_room;
				continue;
			}
			// TODO: UI:显示当前输入模式
			if (get_op->init() == false) {
				const char* str = get_op->getErrorString();
				// TODO: UI:显示错误信息
				get_op->deinit();
				continue;
			}
			if (get_op->loop() == false) {
				if (get_op->getResCode() == Opera::USR_Cancel) {
					get_op->deinit();
					continue;
				} else if (get_op->getResCode() == Opera::OK) {

				} else {
					const char* str = get_op->getErrorString();
					// TODO: UI:显示错误信息
					get_op->deinit();
					continue;
				}
			}
			get_op->deinit();
			// TODO: UI:操作中
			std::list<DB_Usr>::iterator node;
			DB_Sheet* sheet;
			for (int i = 1; i <= 3; i++) {
				DB_Base* base;
				switch (RunningMode) {
				case OP_Finger:
					base = new DB_FingerPrint(get_op->getAns());
					break;
				case OP_RFID:
					base = new DB_RFID(get_op->getAns());
					break;
				case OP_Room:
					base = new DB_Password(get_op->getAns());
					break;
				default:
					base = NULL;
				}
				sheet = new DB_Sheet((enum DB_Sheet::DB_Sector)i);
				node = sheet->search(*base);
				if (node != NULL)
					break;
				delete sheet;
			}

			if (node == NULL) {
				sheet = new DB_Sheet(DB_Sheet::Sector_1);
				DB_RoomID* r = new DB_RoomID(room);
				DB_FingerPrint* finger;
				DB_RFID* rfid;
				DB_Password* pass;
				if (RunningMode == OP_Finger) {
					const uint8_t* ans = get_op->getAns();
					finger = new DB_FingerPrint[5];
					finger[0].overWrite(ans);
					finger[1].overWrite(ans + 2);
					finger[2].overWrite(ans + 4);
					finger[3].overWrite(ans + 6);
					finger[4].overWrite(ans + 8);
				} else {
					finger = new DB_FingerPrint[5];
				}
				if (RunningMode == OP_RFID) {
					rfid = new DB_RFID(get_op->getAns());
				} else {
					rfid = new DB_RFID;
				}
				if (RunningMode == OP_Room) {
					pass = new DB_Password(get_op->getAns());
				} else {
					pass = new DB_Password;
				}
				DB_Usr* usr = new DB_Usr(finger, *rfid, *r, *pass);
				delete r;
				delete finger;
				delete rfid;
				delete pass;
				sheet->add(*usr);
				sheet->writeBack();
				delete sheet;
			} else {
				switch (RunningMode) {
				case OP_Finger: {
					const uint8_t* ans = get_op->getAns();
					node->finger[0].overWrite(ans);
					node->finger[1].overWrite(ans + 2);
					node->finger[2].overWrite(ans + 4);
					node->finger[3].overWrite(ans + 6);
					node->finger[4].overWrite(ans + 8);
				} break;
				case OP_RFID: {
					node->rfid.overWrite(get_op->getAns());

				} break;
				case OP_Room: {
					node->rfid.overWrite(get_op->getAns());
				} break;
				}
				sheet->writeBack();
				delete sheet;
			}
			// TODO: UI:操作成功
		} while (1);
	}
	return false;
}
/*********************************************
 * @name   OP_Handle
 * @brief  操作执行实体
 */
void OP_Handle()
{

	const char* outputStr = NULL;
	uint8_t RunningMode = 0;
	uint8_t SubMode = 0;

	// 接收下位机消息
	keyboard = new STP_KeyMat;
	server = new STP_ServerRS485(&huart6);
	server->reciMessage();

	// TODO: UI:Welcome

	// Waitfor Manager hotkey
	while (!(keyboard->isPress(STP_KeyMat::KEY_ID_0) && keyboard->isPress(STP_KeyMat::KEY_ID_YES)))
		;

	// Setup time
	do {
		// TODO: UI: Set the Time
		Opera* op = new Opera_getUsrKey(*keyboard, Opera_getUsrKey::getTime);
		if (normal_op(op, outputStr) == false) {
			delete op;
			// TODO: OutputString
			continue;
		}
		int ans = atoi((char*)(op->getAns()));
		uint8_t h = ans / 10000;
		uint8_t m = (ans / 100) % 100;
		uint8_t s = (ans % 100);
		rtc->setTime(h, m, s);
		delete op;
	} while (0);

	// Setup Mode
	while (!keyboard->isPress(STP_KeyMat::KEY_ID_YES)) {
		while (keyboard->scan()) {
			if (keyboard->isPress(STP_KeyMat::KEY_ID_UP)) {
				RunningMode = (RunningMode - 1) % OP_Unknow;
			} else if (keyboard->isPress(STP_KeyMat::KEY_ID_DOWN)) {
				RunningMode = (RunningMode + 1) % OP_Unknow;
			}
		}
		// TODO: UI:Update selected mode message
	}
	while (keyboard->scan())
		;

	// 非管理模式-设定子模式
	if (RunningMode < OP_Manager) {
		while (!keyboard->isPress(STP_KeyMat::KEY_ID_YES)) {
			if (keyboard->isPress(STP_KeyMat::KEY_ID_UP)) {
				SubMode -= 1;

			} else if (keyboard->isPress(STP_KeyMat::KEY_ID_DOWN)) {
				SubMode += 1;
			}
			SubMode %= 2;
		}
		// TODO: UI:Update selected mode message
	}
	while (keyboard->scan())
		;
	submode_op(RunningMode, SubMode);
}
/*********************************************
 * @name   Opera_getNFC
 * @brief  获取4字节的RFID
 * @note   过程中响应退出事件
 */
Opera_getNFC::Opera_getNFC(STP_KeyMat& kb)
	: Opera(4)
{
	keymat = &kb;
}
bool Opera_getNFC::init()
{
	nfc = new Adafruit_NFCShield_I2C(
		Adafruit_NFCShield_I2C::STM32_Pin(PN532_RDY_GPIO_Port, PN532_RDY_Pin),
		Adafruit_NFCShield_I2C::STM32_Pin((GPIO_TypeDef*)NULL, 0),
		&hi2c2);
	if (nfc == NULL) {
		ErrorCode = MEM_Error;
		ErrorStr = "Can't offer mem for class:Adafruit_NFCShield_I2C";
		return false;
	}
	nfc->begin();
	if (nfc->getFirmwareVersion() == 0) {
		ErrorCode = DRIVER_Error;
		ErrorStr = "Can't find NFC module";
		return false;
	}
	nfc->setPassiveActivationRetries(0x02);
	nfc->SAMConfig();
	return true;
}
bool Opera_getNFC::deinit()
{
	delete nfc;
	return true;
}
bool Opera_getNFC::exitCheck()
{
	if (keymat->isPress(STP_KeyMat::KEY_ID_NO)) {
		ErrorCode = USR_Cancel;
		ErrorStr = "Usr Cancel";
		return true;
	}
	return false;
}
bool Opera_getNFC::loop()
{
	uint8_t uid[10];
	uint8_t uidLen;
	while (1) {
		if (nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLen)) {
			memcpy(ans, uid, 4);
			return true;
		}
		if (exitCheck()) {
			return false;
		}
	}
}

/*********************************************
 * @name   Opera_getFinger
 * @brief  对比一个指纹或者注册一个新的到指纹库
 * @note   过程中响应退出事件
 */
Opera_getFinger::Opera_getFinger(STP_KeyMat& kb, bool isCheck = true)
	: Opera(2)
	, _isCheck(isCheck)
{
	keymat = &kb;
}
bool Opera_getFinger::init()
{
	uint8_t error = GR307_Init();
	if (error != 0) {
		ErrorCode = DRIVER_Error;
		ErrorStr = GR307_getErrorMsg(error);
		return false;
	}
	return true;
}
bool Opera_getFinger::deinit()
{
	return true;
}
bool Opera_getFinger::exitCheck()
{
	if (keymat->isPress(STP_KeyMat::KEY_ID_NO)) {
		ErrorCode = USR_Cancel;
		ErrorStr = "Usr Cancel";
		return true;
	}
	return false;
}
bool Opera_getFinger::loop()
{
	uint16_t id;
	while (1) {
		if (HAL_GPIO_ReadPin(R307_INT_GPIO_Port, R307_INT_Pin) == GPIO_PIN_RESET) {
			uint8_t error = 0;
			if (_isCheck) {
				error = GR307_Check(&id);
			} else {
				error = GR307_Register(&id);
			}
			if (error != 0) {
				ErrorCode = DRIVER_Error;
				ErrorStr = GR307_getErrorMsg(error);
				return false;
			}
			memcpy(ans, &id, 2);
			return true;
		}
		if (exitCheck()) {
			return false;
		}
	}
}

Opera_getUsrKey::Opera_getUsrKey(STP_KeyMat& kb, enum getMode mode)
	: Opera((int)mode)
	, _mode(mode)
	, maxNum((int)mode)
{
	keymat = &kb;
	pos = 0;
}
bool Opera_getUsrKey::init()
{
	memset(ans, 0, maxNum);
	return true;
}
bool Opera_getUsrKey::deinit()
{
	return true;
}
bool Opera_getUsrKey::exitCheck()
{
	if (keymat->isPress(STP_KeyMat::KEY_ID_NO)) {
		ErrorCode = USR_Cancel;
		ErrorStr = "Usr Cancel";
		return true;
	} else if (keymat->isPress(STP_KeyMat::KEY_ID_YES) && pos >= maxNum) {
		ErrorCode = OK;
		ErrorStr = "Comfirme";
		return true;
	} else if (keymat->isPress(STP_KeyMat::KEY_ID_YES) && pos < maxNum) {
		ErrorCode = USR_Error;
		ErrorStr = "Please input enough char";
		return true;
	}
	return false;
}
bool Opera_getUsrKey::loop()
{
	pos = 0;
	while (1) {
		if (keymat->scan()) {
			if (keymat->isPress(STP_KeyMat::KEY_ID_0)) {
				ans[pos++] = '0';
			} else if (keymat->isPress(STP_KeyMat::KEY_ID_1)) {
				ans[pos++] = '1';
			} else if (keymat->isPress(STP_KeyMat::KEY_ID_2)) {
				ans[pos++] = '2';
			} else if (keymat->isPress(STP_KeyMat::KEY_ID_3)) {
				ans[pos++] = '3';
			} else if (keymat->isPress(STP_KeyMat::KEY_ID_4)) {
				ans[pos++] = '4';
			} else if (keymat->isPress(STP_KeyMat::KEY_ID_5)) {
				ans[pos++] = '5';
			} else if (keymat->isPress(STP_KeyMat::KEY_ID_6)) {
				ans[pos++] = '6';
			} else if (keymat->isPress(STP_KeyMat::KEY_ID_7)) {
				ans[pos++] = '7';
			} else if (keymat->isPress(STP_KeyMat::KEY_ID_8)) {
				ans[pos++] = '8';
			} else if (keymat->isPress(STP_KeyMat::KEY_ID_9)) {
				ans[pos++] = '9';
			} else if (keymat->isPress(STP_KeyMat::KEY_ID_LEFT)) {
				if (pos > 0) {
					ans[pos--] = '\0';
				}
			}
		}
		if (exitCheck()) {
			return false;
		}
	}
}
