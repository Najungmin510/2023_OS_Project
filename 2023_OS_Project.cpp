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
#define MAX_JOIN_USER 10 //최대 동시접속가능수

/*
GUI를 구현하기 위해 Easywin32 라이브러리를 사용
그러나, 이 라이브러리에서 socket을 이용하려면 개발자가 따로 선언 한
함수 형식에 따라야 하기 때문에, 그에 맞춰서 구현함
★ sprintf_s는 문자열을 복사하는 역할을함!!!! <<<<
*/

struct UserData;

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
	int user_Count; //서버 접속자 수 저장할 변수
};


struct UserData { //서버에 접속하는 유저의 객체 생성해서 저장할 struct, 각 유저를 구분해야 하니까
	unsigned int h_socket; //socket handle
	char ip_Address[16]; //사용자의 클라이언트 주소
	//이 두가지 기능은 서버 프로그램에서 반드시 필요한 요소
};

//유저 등급, 바뀌질 않을 데이터라 걍 전역변수로 
const char* gp_user_level_str[5] = { "초보자", "숙련자", "고수", "달인", "관리자" };

/*----------------------------------------------------------------------*/


/* 1. 새로운 사용자 접속 시 서버에서 처리할 것들 */
/* a_server_index의 경우 다중 서버 구현 시, 어떤 서버로 데이터를 보낼것인지를 지정할 때 사용 */
void OnNewUser(UserData* ap_user_data, void *ap_server, int a_server_index) { //EasyWin32에서 socket사용할 때 필요한 함수형식
	char temp_str[64];
	sprintf_s(temp_str, 64, "환영합니다! %s 님이 접속하셨습니다.", ap_user_data->ip_Address);
	ListBox_InsertString(FindControl(ID_LB_CHAT), -1, temp_str);

	//메인에서 만든 UserBoard를 가져오고, 새로운사용자가 접속하면 그걸 배열의 맨 뒤에다가 추가
	//위의 환영합니다 문구를 추가하는거, 그리고 새로운 사용자가 들어올 때 마다 이를 출력한다.	
	
}

//게임에 참여하고 있는 플레이어 수 계산
int CalcPlayerCount() {

	AppData* ap_app_data = (AppData*)GetAppData();
	int before_join = ap_app_data->user_Count; // 기존 유저 수 확인

	ap_app_data->user_Count = 0;//플레이어 수 초기화

	UserData* p = (UserData*)GetUsersData(ap_app_data->p_server), * p_limit = p + MAX_JOIN_USER;

	while (p < p_limit) {
		if (p->h_socket != 0xFFFFFFFF) { //접속된 사용자라면
			ap_app_data->user_Count++;
		}
		p++;
	}

	if (ap_app_data->user_Count >= 2) { //만약, 접속된 사용자수가 2명이상이라면 게임시작가능
		ListBox_InsertString(ap_app_data->p_chat_list, -1, "[알림] : 2명 이상의 사용자가 참가하여, 그림퀴즈 게임 이용이 가능합니다!");
		return 1; // 1 == 게임시작가능
	}
	else {
		ListBox_InsertString(ap_app_data->p_chat_list, -1, "[알림] : 2명 미만의 사용자가 참가하여, 그림퀴즈 게임 이용이 불가능합니다!");
		return -1; // -1 == 게임시작불가
	}
}

//그림퀴즈,  
void PlayDrawQuizGame() { //사용자 & 단어 랜덤으로 지명해서 클라이언트로 보내기

	AppData* p_app_data = (AppData*)GetAppData();
	int Random_Myturn = p_app_data->user_Count; //접속인원 수 구하기

	int who = rand() % (Random_Myturn - 0 + 1) + 0; //접속인원 중 무작위로 뽑는 난수
	int i = 0; // 내가 찾는 유저가 맞는지 카운트할 변수

	UserData* p = (UserData*)GetUsersData(p_app_data->p_server), * p_limit = p + MAX_JOIN_USER;

	//랜덤하게 사용자 한명 선택해서 방장(그림을 그려야하는 사람)으로 정한 후, 그 사람 클라이언트로 단어 아무거나 하나 보내기
	while (i != who) { //둘이 같을때까지
		p -> h_socket; //타겟 돌리기
		i++; // 랜덤으로 뽑힌 수까지 가기위해서 증가시킴
	}
	//탈출했다는 건 i == who 이므로
	if (i == who) {
		ListBox_InsertString(FindControl(1000), -1, "[알림] : 랜덤하게 선택된 유저에게 단어를 보냈습니다! 개인 채팅창을 확인해주세요.");
		ListBox_InsertString(FindControl(1000), -1, "[알림] : 해당 유저가 그리는 그림을 보고, 정답이라 생각되는 단어를 적어주세요. ex) 호랑이");

		//형식 : void SendFrameDataToClient(서버소켓, 클라이언트 소켓, 데이터 구분번호, 전송 데이터 주소, 전송 데이터 크기)
		SendFrameDataToClient(p_app_data->p_server, p->h_socket, 12, NULL, 0);
		/*랜덤하게 뽑힌 사람이라면, 그 유저에게 데이터를 전달하라고, 함수호출
		그냥보내주고 클라이언트에서 가공해서 listbox에서 띄우면됨*/
	}
}

//정답인 단어 가지고 와서 맞춘 유저가 있는지 확인하는 기능, (이 방법 아닌것같아서 보류)
//void CheckAnswer(char *answerWord) { //단어 가져오고
	//AppData* p_data = (AppData*)GetAppData();
	//UserData* ap_user_data = (UserData*)ap_data->mp_net_body_data;
	//ListBox_InsertString(FindControl(ID_LB_CHAT), -1, temp_str); //디버깅 용도로 만들었던 것, 값 잘 나오는거 확인
	//ListBox_InsertString(FindControl(ID_LB_CHAT), -1, "정답자가 나왔습니다!");
	//PlayDrawQuizGame();//정답자가 있으면 재추첨해서 게임 진행
//}

const char* saveWord; // 정답확인용 변수

// 2. 클라이언트에서 온 데이터 처리 및 데이터 보낼 때 사용하는 함수 
int OnClientMessage(CurrentServerNetworkData *ap_data, void *ap_server, int a_server_index) {
	/*CurrentServer... 자체에 UserData가 포함이 되어 있는 구조라서 따로 호출 안해도 된다.*/
	//<강의 중요 코멘트>
	//전달된 사용자 정보를 자신이 선언한 구조체로 형 변환해서 사용하면 된다
	//서버 구현 시 Sizeof(UserData)크기로 만들어 달라 하였기에 사용자 정보 : UserData 형식으로 관리되고 있음

	UserData *p_user_data = (UserData *)ap_data->mp_net_user; //현재 유저의 데이터 받아와서

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

		if (strcmp(saveWord, temp_str)) { //사용자가 말한 답과, 클라이언트상에서 지정해준 답이 같다면 정답처리
			ListBox_InsertString(FindControl(ID_LB_CHAT), -1, "------정답자------");
			ListBox_InsertString(FindControl(ID_LB_CHAT), -1, p_user_data->ip_Address);
			ListBox_InsertString(FindControl(ID_LB_CHAT), -1, saveWord);
			ListBox_InsertString(FindControl(ID_LB_CHAT), -1, "------------------");

			PlayDrawQuizGame(); //그리고 게임 계속 진행
		}

	}
	else if (ap_data -> m_net_msg_id == 2 || ap_data -> m_net_msg_id == 3) {
		//선 그리기 : 2번, 지우기 : 3번, 그냥 그대로 클라이언트로 보내면 됨
		BroadcastFrameData(ap_server, ap_data->m_net_msg_id, ap_data->mp_net_body_data, ap_data->m_net_body_size);
	}
	else if (ap_data->m_net_msg_id == 11) { //게임 시작 id : 11, 인원 수 확인먼저, 2명이상이면 1, 아니면 -1리턴됨
		int check = CalcPlayerCount(); // 서버 접속 인원수 확인하는 함수 호출

		if (check == 1) { //만약 인원수가 2명이상이라면
			ListBox_InsertString(FindControl(ID_LB_CHAT), -1, "[알림] : 그림퀴즈 게임 시작 버튼이 눌렸습니다!");
			PlayDrawQuizGame(); //그림퀴즈 시작 함수 호출
		}
	}
	else if (ap_data->m_net_msg_id == 100) { //정답확인 id : 100, 클라이언트에서 선택한 정답을 가지고 와서, 사용자들이 입력한 것과 같다면
		//게임 함수 재호출하기.
		sprintf_s(temp_str, 128, "%s", ap_data->mp_net_body_data); //값 복사해 가져와서
		//ListBox_InsertString(FindControl(ID_LB_CHAT), -1, temp_str); //디버깅 용도로 만들었던 것, 값 잘 나오는거 확인
		saveWord = temp_str; // 정답 값 저장해주고
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

// 이미 다른 사용자가 사용하고 있는 아이디인지 확인하는 기능
RUD* FindUserID(AppData* ap_data, const char* ap_user_id) {
	RUD* p_user_info;
	int count = ListBox_GetCount(ap_data->p_user_list); //리스트 박스에 있는 항목의 수만큼 돌면 되니까 이거 개수 가져오고

	for (int i = 0; i < count; i++) {
		p_user_info = (RUD*)ListBox_GetItemDataPtr(ap_data->p_user_list, i); //i번째 항목에 저장된 사용자 주소 가져오기 

		if (!strcmp(p_user_info->id, ap_user_id)) { //만약 존재하는 아이디라면
			return p_user_info; // strcmp 같으면 0을 반환하기에 현재 사용자의 주소를 반환 (존재함을 알려주는 역할)
		}
	}
	return NULL; //만약 없는 아이디라면 null 리턴
}

// Edit 컨트롤에 입력된 문자열 사용자 지정 메모리에 복사하는 기능
void CopyControlDataToMemory(int a_ctrl_id, char* ap_memory, int a_mem_size) {
	void* p_edit = FindControl(a_ctrl_id); //edit 컨트롤 주소 가져온다음에
	GetCtrlName(p_edit, ap_memory, a_mem_size); //거기에 있는 text를 메모리에 복사
	SetCtrlName(p_edit, ""); //복사 후, eidt 컨트롤에 남아있는 텍스트 삭제
}

// 사용자가 입력한 데이터를 가져와 저장하는 기능
void RegisteringUserData() {

	AppData* p_data = (AppData *)GetAppData(); //이 프로그램의 내부 데이터 주소를 가져오기
	char str[32];
	void* p_edit = FindControl(ID_EDIT_ID); //아이디를 입력받은 edit 컨트롤 주소 가져오기 

	GetCtrlName(p_edit, str, 32); // str 변수에 이를 복사하기

	int id_len = strlen(str) + 1; //입력된 아이디 길이를 구할건데

	if (id_len > 5) { //5자 이상이라면
		if (NULL == FindUserID(p_data, str)) { //이미 사용되고 있는 아이디가 있는지 조회, 없다면(Null이 반환되었다면)
			SetCtrlName(p_edit, ""); // 아이디 사용가능, 저장 후 edit 컨트롤에 있는 문자열 부분 지워주기

			RUD* p_temp_user = (RUD*)malloc(sizeof(RUD)); //사용자 정보를 저장할 메모리 , 동적 메모리 할당
			if (p_temp_user != NULL) { //만약 할당 되었다면,
				memcpy(p_temp_user->id, str, id_len); //메모리 복사해서 사용자가 입력한 값을 넣어주기
				CopyControlDataToMemory(ID_EDIT_PASSWORD, p_temp_user -> pw, 32); //비밀번호
				CopyControlDataToMemory(ID_EDIT_NICKNAME, p_temp_user -> nickname, 32); //닉네임 가져와서 복사

				p_temp_user -> level = ComboBox_GetCurSel(FindControl(ID_CB_LEVEL)); //콤보박스의 현재 위치(인덱스)가져와서 저장해주면 됨, 배열로 만들어 뒀으니까
				p_temp_user -> p_socket = NULL; //사용자 접속 정보를 초기화

				int index = ListBox_AddString(p_data -> p_user_list, p_temp_user -> id, 0); //사용자 목록 리스트박스에 저장, 아이디만 추가해주기
				ListBox_SetItemDataPtr(p_data -> p_user_list, index, p_temp_user); //현재 추가된 위치에 사용자의 새 정보를 p_temp_user에 복사
				//즉, 메모리 남은 부분에 사용자에게 입력받은 데이터 저장해두겠다는 것
				ListBox_SetCurSel(p_data -> p_user_list, index); //추가된 항목을 바로 볼 수 있도록 해당 박스 커서 설정
			}
		}
		else { //이미 사용하고 있는 아이디가 있다면, 관련 안내 메세지 출력
			ListBox_InsertString(p_data -> p_chat_list, -1, "[등록 오류] : 이미 사용중인 아이디입니다.");
		}
	}
	else { //5글자 이하라면, 관련 안내 메세지 출력
		ListBox_InsertString(p_data->p_chat_list, -1, "[등록 오류] : 아이디를 5글자 이상 입력해주세요.");
	}
}

void OnCommand(INT32 a_ctrl_id, INT32 a_notity_code, void* ap_ctrl) { //사용자가 버튼을 클릭하였을 때 행동 정의
	if (a_ctrl_id == ID_BTN_ADD) { //추가 버튼 클릭시
		RegisteringUserData();
	}
}

//사용자 목록에 등록된 정보를 기억하는 기능 , DB역할이라고 보면 된다.
void SaveUserData(const char* ap_file_name) {
	FILE* p_file = NULL;

	if (0 == fopen_s(&p_file, ap_file_name, "wb") && p_file != NULL) {
		void* p_ctrl = FindControl(ID_LB_USER);
		int count = ListBox_GetCount(p_ctrl); //리스트 박스에 추가되어 있는 항목 개수 가져오고 이게 유저수와 동일하므로
		fwrite(&count, sizeof(int), 1, p_file); //유저수를 파일에 저장, 나중에 읽기에 좀 더 편하려고 저장한거

		for (int i = 0; i < count; i++) { //그래서그 수만큼 읽어오면 됨
			fwrite(ListBox_GetItemDataPtr(p_ctrl, i), sizeof(RUD), 1, p_file);
			/*리스트 박스에 사용자 관련 메모리를 저장해 두었기 때문에 거기에 있는 데이터 가져와서
			파일에 저장*/
		}
		fclose(p_file); //파일닫기
	}
}

void DestoryUserData() { //프로그램 종료시, 사용자 할당 메모리 자동 해제 하도록 하는 기능

	//근데, 회원가입을 한 사용자의 정보가 영원히 사라지면 안되기에, 이를 따로 저장하는 기능을 추가해야함
	SaveUserData("user_list.dat");
	//사용자 목록에 등록된 정보 파일로 저장해서 관리
	//이 파일은 overwrite됨 , 그냥 지우고 다시 저장하는 방식

	void* p_data, * p_ctrl = FindControl(ID_LB_USER); //사용자 목록 리스트 박스 가져오기
	int count = ListBox_GetCount(p_ctrl); //리스트 박스에 있는 항목 개수 가져오고

	for (int i = 0; i < count; i++) { //그 항목 개수만큼 돌면서
		p_data = ListBox_GetItemDataPtr(p_ctrl, i); //i번째 항목에 저장된 주소 가져오기
		free(p_data); //p_data가 가리키는 메모리 할당 해제
	}
	ListBox_ResetContent(p_ctrl); //리스트 박스의 모든 항목 제거
}


//컨트롤 조작 시 OnCommand 함수 호출 및 종료될 시 Destory함수 호출, 각 함수는 매크로
//Destory 같은 경우 동적 할당 된 메모리들을 정리할 수 있는 시간을 주는 역할을 함
CMD_USER_MESSAGE(OnCommand, DestoryUserData, NULL) //사용자 정의 메세지 사용


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
	FILE* p_file = NULL;

	if (0 == fopen_s(&p_file, ap_file_name, "rb") && p_file != NULL) {
		int count = 0, index;
		fread(&count, sizeof(int), 1, p_file); //파일에 저장되어 있는 사용자수 읽기
		RUD* p_user_info;

		for (int i = 0; i < count; i++) {
			p_user_info = (RUD*)malloc(sizeof(RUD));//사용자 정보를 저장할 메모리 할당

			if (p_user_info != NULL) { //사용자 전부 입력될 때 까지
				fread(p_user_info, sizeof(RUD), 1, p_file); //1명의 사용자 정보를 읽어들이기
				p_user_info->p_socket = NULL; //socket 쓰레기값 있을수도 있기에 NULL로 초기화

				index = ListBox_AddString(ap_data -> p_user_list, p_user_info->id, 0); //사용자 ID로 정렬될 수 있도록 하기위해서 추가
				//정렬은 AddString에 있는 기본기능임

				ListBox_SetItemDataPtr(ap_data -> p_user_list, index, p_user_info); // 새로 추가된 항목에 사용자 정보 & 주소 함께 저장
			}
		}
		fclose(p_file);
	}
}



void CreateUI(AppData *ap_data) {
	SelectFontObject("굴림", 12);
	TextOut(15, 10, RGB(200, 255, 200), "전체 사용자 채팅글"); //제목
	ap_data->p_chat_list = CreateListBox(10, 30, 600, 200, ID_LB_CHAT);


	TextOut(15, 243, RGB(200, 255, 200), "채팅방 유저 목록");
	ap_data->p_user_list = CreateListBox(10, 263, 600, 160,
		ID_LB_USER, DrawUserDataItem); //사용자가 리스트 박스를 재정의 할 수 있게하기 위해서 선언

	CreateButton("추가", 207, 430, 70, 28, ID_BTN_ADD); // 각 기능을 하는 버튼들 생성
	CreateButton("변경", 280, 430, 70, 28, ID_BTN_MODIFY);
	CreateButton("삭제", 353, 430, 70, 28, ID_BTN_DEL);

	/* 사용자 정보 출력*/
	TextOut(15, 476, RGB(255, 255, 255), "아이디 : ");
	CreateEdit(65, 470, 100, 24, ID_EDIT_ID, 0);

	TextOut(180, 476, RGB(255, 255, 255), "암호 : ");
	CreateEdit(218, 470, 100, 24, ID_EDIT_PASSWORD, 0);

	TextOut(333, 476, RGB(255, 255, 255), "닉네임 : ");
	CreateEdit(371, 470, 100, 24, ID_EDIT_NICKNAME, 0);

	TextOut(486, 476, RGB(255, 255, 255), "등급 : ");
	void* p = CreateComboBox(524, 470, 76, 186, ID_CB_LEVEL);
	for (int i = 0; i < 5; i++) {
		ComboBox_InsertString(p, i, gp_user_level_str[i], 0); //5개 문자열 넣어주고
	}
	ComboBox_SetCurSel(p, 0); //기본값으로 초보자 
}

int main()
{
	ChangeWorkSize(620, 650); //작업화면 크기 설정 
	Clear(0, RGB(41, 22, 77)); //서버 메인 화면 설정 
	SelectFontObject("굴림", 15);
	//TextOut(10, 5, RGB(151, 125, 255), "채팅목록"); //제목 설정

	/*여기서부터 socket code*/

	StartSocketSystem(); //Socket 사용

	AppData data;

	//(유저 수,새로운 사용자 등장시, 클라이언트의 반응, 사용자 접속 해제 시)
	data.p_server = CreateServerSocket(sizeof(UserData), OnNewUser, OnClientMessage, OnCloseUser);

	CreateUI(&data); //관련 컨트롤 생성
	SetAppData(&data, sizeof(AppData)); //지정한 변수를 내부 데이터로 저장
	LoadUserData(&data, "user_list.dat"); //저장되어 있던 사용자 목록 가져오기

	StartListenService(data.p_server, "127.0.0.1", 25001); //25001포트를 이용해 서버를 실행시키기

	ShowDisplay();
	return 0;

}