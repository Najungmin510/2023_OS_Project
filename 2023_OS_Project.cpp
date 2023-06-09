#include "pch.h"
#include <stdio.h>
#include "tipsware.h"

#define ID_LB_CHAT 1000  //채팅용 리스트박스, id = 1000로 정의
#define ID_LB_USER 1001 //사용자 목록이 저장 될 리스트 박스 

#define ID_BTN_ADD 1011 //추가 버튼
#define ID_BTN_MODIFY 1012 //변경 버튼
#define ID_BTN_DEL 1013 //삭제 버튼

#define ID_EDIT_ID 1020 //아이디 입력 받은 text id
#define ID_EDIT_PASSWORD 1021 //비밀번호 입력받은 text id
#define ID_EDIT_NICKNAME 1022 //닉네임 입력받은 text id

#define ID_CB_LEVEL 1030 //등급 입력용 콤보 박스

/*
GUI를 구현하기 위해 Easywin32 라이브러리를 사용
그러나, 이 라이브러리에서 socket을 이용하려면 개발자가 따로 선언 한
함수 형식에 따라야 하기 때문에, 그에 맞춰서 구현함
*/

//컨트롤 조작 시 OnCommand 함수 호출 및 종료될 시 Destory함수 호출, 각 함수는 매크로
//Destory 같은 경우 동적 할당 된 메모리들을 정리할 수 있는 시간을 주는 역할을 함
CMD_USER_MESSAGE(OnCommand, DestoryUserData, NULL) //사용자 정의 메세지 사용

typedef struct RegisteredUserData { // 서버에 접속하는 사용자를 "관리"하기 위한 구조체, 유저 한명당 하나씩 생성
	char id[32]; // 사용자 id
	char pw[32]; // 사용자 pw
	char nickname[32]; //별명
	char level; //콤보박스 값
	UserData* p_socket;// 접속중이라면 사용중인 소켓 정보의 주소
}RUD; //이 구조체의 약자


struct AppData { //서버에서 사용할 내부 데이터
	void* p_user_list; //사용자 목록 리스트 박스의 주소
	void* p_chat_list; //채팅 목록 리스트 박스의 주소 
	void* p_server; //서버 소켓 객체의 주소
};


struct UserData { //서버에 접속하는 유저의 객체 생성해서 저장할 struct, 각 유저를 구분해야 하니까
	unsigned int h_socket; //socket handle
	char ip_Address[16]; //사용자의 클라이언트 주소
	//이 두가지 기능은 서버 프로그램에서 반드시 필요한 요소
};

//유저 등급, 바뀌질 않을 데이터라 걍 전역변수로 
const char* gp_user_level_str[5] = { "초보자", "숙련자", "고수", "달인", "관리자" };

/* 1. 새로운 사용자 접속 시 서버에서 처리할 것들 */
/* a_server_index의 경우 다중 서버 구현 시, 어떤 서버로 데이터를 보낼것인지를 지정할 때 사용 */
void OnNewUser(UserData* ap_user_data, void *ap_server, int a_server_index) { //EasyWin32에서 socket사용할 때 필요한 함수형식

	char temp_str[64];
	sprintf_s(temp_str, 64, "환영합니다! %s 님이 접속하셨습니다.", ap_user_data->ip_Address);
	ListBox_InsertString(FindControl(ID_LB_CHAT), -1, temp_str);
	//메인에서 만든 UserBoard를 가져오고, 새로운사용자가 접속하면 그걸 배열의 맨 뒤에다가 추가
	//위의 환영합니다 문구를 추가하는거, 그리고 새로운 사용자가 들어올 때 마다 이를 출력한다.
}

// 2. 클라이언트로 메세지를 보낼 때 처리할 것들
int OnClientMessage(CurrentServerNetworkData *ap_data, void *ap_server, int a_server_index) {
	/*CurrentServer... 자체에 UserData가 포함이 되어 있는 구조라서 따로 호출 안해도 된다.*/

	UserData *p_user_data = (UserData *)ap_data->mp_net_user; //현재 유저의 데이터
	char temp_str[128];

	// 프로그램에서 각 기능을 구분하기 위해서 id를 부여해줌 
	// ex) 채팅같은 경우는 id 1, 대화 삭제하기 같이 경우는 2.. 이런씩으로 id 확인 후 관련 기능 수행

	if (ap_data->m_net_msg_id == 1) { //채팅 메세지의 경우 id = 1로 선언해두었음

		sprintf_s(temp_str, 128, "%s : %s", p_user_data->ip_Address, ap_data->mp_net_body_data);//사용자 ip, 사용자의 채팅 데이터 전송
		ListBox_InsertString(FindControl(ID_LB_CHAT), -1, temp_str); //사용자각 보낸 데이터를 배열의 맨 뒤에 붙임, 즉 대화가 아래로 내려갈 수 있도록 구성하는거

		/*클라이언트 -> 서버로 그냥 사용자가 입력한 데이터를 전송만함
		그리고 이제 데이터가 다른 유저에게 전송될 수 있도록 하는 것이 BroadCast
		즉, 네트워크 상으로 자신의 데이터가 들어왔을 때 저장 후 재전송해 나를 포함한 모두가 채팅을 볼 수 있게 하는것*/
		BroadcastFrameData(ap_server, 1, temp_str, strlen(temp_str) + 1);
	}
	return 1;
}

// 3. 나가기 관련 이벤트
void OnCloseUser(UserData* ap_user_data, void* ap_server, int a_error_flag, int a_server_index) {
	char temp_str[64];

	if (a_error_flag == 1) { //사용자가 정상적이지 않은 방법으로 채팅 연결 시도시
		sprintf_s(temp_str, 64, "%s 사용자가 올바르지 않은 경로로 접근하여, 강제탈퇴합니다.", ap_user_data->ip_Address);
		//관련 안내 문구를 출력해주기
	}
	else {
		sprintf_s(temp_str, 64, "%s 사용자가 채팅방에서 나갔습니다.", ap_user_data->ip_Address);
	}
	ListBox_InsertString(FindControl(ID_LB_CHAT), -1, temp_str);
}


void RegisteringUserData() {

	AppData* p_data = (AppData*)GetAppData();
	char str[32];
	void* p_edit = FindControl(ID_EDIT_ID);

	GetCtrlName(p_edit, str, 32);

	int id_len = strlen(str) + 1;
	if (id_len > 5) {
		if (NULL == FindUserID(p_data, str)) {
			SetCtrlName(p_edit, "");

			RUD* p_temp_user = (RUD*)malloc(sizeof(RUD));

			if (p_temp_user != NULL) {
				memcpy(p_temp_user->id, str, id_len);
				CopyControlDataToMemory(ID_EDIT_PASSWORD, p_temp_user->pw, 32);
				CopyControlDataToMemory(ID_EDIT_NICKNAME, p_temp_user->pw, 32);
			}
		}
	}
}

void OnCommand(INT32 a_ctrl_id, INT32 a_notity_code, void* ap_ctrl) {
	if (a_ctrl_id == ID_BTN_ADD) { //추가 버튼 클릭시
		RegisteringUserData();
	}
}


//콤보 상자 항목 재정의
void DrawUserDataItem(int index, char* ap_str, int a_str_len, void* ap_data, int a_is_selected, RECT* ap_rect) {
	if (a_is_selected) {
		SelectPenObject(RGB(200, 255, 255)); //선택된 항목은 색깔 다르게 표시해주기
	}
	else {
		SelectPenObject(RGB(62, 77, 104)); //아니라면 기본 상태 유지
	}

	SelectBrushObject(RGB(62, 77, 104)); //각 항목의 기본 배경색 & 사각형 지정
	Rectangle(ap_rect->left, ap_rect->top, ap_rect->right, ap_rect->bottom);

	RUD* p_data = (RUD*)ap_data;
	SelectFontObject("굴림", 12);

	//사용자 아이디 & 이름 출력
	TextOut(ap_rect->left + 5, ap_rect->top + 3, RGB(255, 255, 0), "%s(%s)", p_data->id, p_data->nickname);
	//사용자 등급 출력
	TextOut(ap_rect->left + 200, ap_rect->top + 3, RGB(255, 255, 0), gp_user_level_str[p_data->level]);

	if (p_data -> p_socket != NULL) { //만약 접속 상태일 시 ip도 표시하도록 함
		TextOut(ap_rect->left + 300, ap_rect->top + 3, RGB(200, 212, 225), p_data->p_socket->ip_Address);
	}
}

// 시스템에 등록된 사용자 정보를 파일에서 읽어 리스트 박스에 추가하는 함수
void LoadUserData(AppData* ap_data, const char* ap_file_name) {

}

void CreateUI(AppData *ap_data) {
	SelectFontObject("굴림", 12);
	TextOut(15, 10, RGB(200, 255, 200), "사용자 채팅글 목록"); //제목
	ap_data->p_chat_list = CreateListBox(10, 30, 600, 200, ID_LB_CHAT);


	TextOut(15, 243, RGB(200, 255, 200), "등록된 유저");
	ap_data->p_user_list = CreateListBox(10, 263, 600, 160,
		ID_LB_USER, DrawUserDataItem); //사용자가 리스트 박스를 재정의 할 수 있게하기 위해서 선언

	CreateButton("추가", 207, 430, 70, 28, ID_BTN_ADD); // 각 기능을 하는 버튼들 생성
	CreateButton("변경", 207, 430, 70, 28, ID_BTN_MODIFY);
	CreateButton("삭제", 207, 430, 70, 28, ID_BTN_DEL);

	/* 사용자 정보 출력*/
	TextOut(15, 476, RGB(255, 255, 255), "아이디 : ");
	CreateEdit(65, 470, 100, 24, ID_EDIT_ID, 0);

	TextOut(180, 476, RGB(255, 255, 255), "암호 : ");
	CreateEdit(65, 470, 100, 24, ID_EDIT_PASSWORD, 0);

	TextOut(333, 476, RGB(255, 255, 255), "닉네임 : ");
	CreateEdit(65, 470, 100, 24, ID_EDIT_NICKNAME, 0);

	TextOut(15, 476, RGB(255, 255, 255), "등급 : ");
	void* p = CreateComboBox(524, 470, 76, 186, ID_CB_LEVEL);
	for (int i = 0; i < 5; i++) {
		ComboBox_InsertString(p, i, gp_user_level_str[i], 0); //5개 문자열 넣어주고
	}
	ComboBox_SetCurSel(p, 0); //기본값으로 초보자 
}

int main()
{
	ChangeWorkSize(490, 650); //작업화면 크기 설정 
	Clear(0, RGB(41, 22, 77)); //서버 메인 화면 설정 
	SelectFontObject("굴림", 15);
	TextOut(10, 5, RGB(151, 125, 255), "채팅목록");

	/*여기서부터 socket*/

	StartSocketSystem(); //Socket 사용

	AppData data;

	//(유저 수,새로운 사용자 등장시, 클라이언트의 반응, 사용자 접속 해제 시)
	data.p_server = CreateServerSocket(sizeof(UserData), OnNewUser, OnClientMessage, OnCloseUser);

	CreateUI(&data); //관련 컨트롤 생성
	SetAppData(&data, sizeof(AppData)); //지정한 변수를 내부 데이터로 저장

	StartListenService(data.p_server, "127.0.0.1", 25001); //25001포트를 이용해 서버를 실행시키기

	ShowDisplay();
	return 0;

}