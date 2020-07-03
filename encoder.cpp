#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <queue>

using namespace std;

// Context Adaptive Node
typedef struct AdaptiveHuffmanTreeNode {
	string encoded;
	int freq;
	unsigned char preceding_symbol;	// preceding_symbol	: X-1
	unsigned char ascii_code;		// current_symbol	: X
	AdaptiveHuffmanTreeNode* leftnode;
	AdaptiveHuffmanTreeNode* rightnode;
} AdaptiveHuffmanTreeNode;


// Huffman TreeNode
typedef struct HuffmanTreeNode {
	string encoded;	//인코딩 된 수
	int freq;		//빈도수
	unsigned char ascii_code;	//ASCII CODE
	HuffmanTreeNode* leftnode;	//왼쪽 노드 
	HuffmanTreeNode* rightnode;	//오른쪽 노드
} HuffmanTreeNode;


// Insert Normal Huffman TreeNode
HuffmanTreeNode* HuffmanTree_Insert(HuffmanTreeNode* u, HuffmanTreeNode* v) {
	HuffmanTreeNode *pt;
	pt = new HuffmanTreeNode;	//부모 노드 설정
	pt->ascii_code = NULL;
	pt->freq = u->freq + v->freq;	//빈도수를 합한다.
	pt->leftnode = u;			//좌, 우 노드 설정
	pt->rightnode = v;
	return pt;		//부모노드 반환
}


// Insert Context Adaptive Huffman TreeNode
AdaptiveHuffmanTreeNode* Adaptive_insert(AdaptiveHuffmanTreeNode* au, AdaptiveHuffmanTreeNode* av) {
	AdaptiveHuffmanTreeNode *apt;
	apt = new AdaptiveHuffmanTreeNode;	//부모 노드 설정
	apt->preceding_symbol = NULL;
	apt->ascii_code = NULL;
	apt->freq = au->freq + av->freq;	//빈도수를 합한다.
	apt->leftnode = au;			//좌, 우 노드 설정
	apt->rightnode = av;
	return apt;		//부모노드 반환
}


// Create Normal Huffman Table
int MakeTable_Normal(HuffmanTreeNode* t, int &tnum) {
	FILE * huff_fp = fopen("huffman_table.hbs", "r+b");	//이어쓰기
	if (huff_fp == NULL)	huff_fp = fopen("huffman_table.hbs", "a+b");	//존재하지 않는다면 생성
	unsigned char bit_length = t->encoded.length();	//string의 길이 == encoded bit length
	unsigned char * temp_c;
	temp_c = new unsigned char[bit_length + 1];
	strcpy((char *)temp_c, t->encoded.c_str());		//string to unsigned char arr
	if (tnum != 0) {
		unsigned char ascii_hi = t->ascii_code >> tnum;
		unsigned char ascii_lo = t->ascii_code << (8 - tnum);	//ascii 자름
		unsigned char bit_length_hi = bit_length >> tnum;
		unsigned char bit_length_lo = bit_length << (8 - tnum);	//bitlength 자름
		fseek(huff_fp, 0, SEEK_END);
		fseek(huff_fp, -1L, SEEK_CUR);
		unsigned char last_char = fgetc(huff_fp);
		unsigned char temp = NULL;
		fseek(huff_fp, -1L, SEEK_CUR);
		temp += last_char;
		temp += ascii_hi;
		fprintf(huff_fp, "%c", temp);
		fprintf(huff_fp, "%c", ascii_lo + bit_length_hi);
		bool end_flag = false;
		int codeword_count = 0;
		for (int i = 0; i < 8 - tnum; i++) {	//codeword 앞부분
			if (temp_c[codeword_count] == NULL) {
				end_flag = true;
				break;
			}
			if ((temp_c[codeword_count] & 0x01) == 0x01) {
				bit_length_lo += 0x01 << (7 - tnum - i);
			}
			else {
				bit_length_lo += 0x00 << (7 - tnum - i);
			}
			codeword_count++;
		}
		fprintf(huff_fp, "%c", bit_length_lo);
		int temp_cc = codeword_count;
		if (end_flag == false) {
			unsigned char codeword_lo = NULL;
			for (int j = 0; j < (bit_length - temp_cc - 1) / 8 + 1; j++) {	//codeword뒷부분 + stuffing bit
				codeword_lo = NULL;
				if (temp_c[codeword_count] == NULL) break;
				for (int i = 0; i < 8; i++) {
					if (temp_c[codeword_count] == NULL) break;
					if (temp_c[codeword_count] & 0x01 == 0x01) {
						codeword_lo += 0x01 << (7 - i);
					}
					else {
						codeword_lo += 0x00 << (7 - i);
					}
					codeword_count++;
				}
				fprintf(huff_fp, "%c", codeword_lo);
			}
		}
	}
	else {
		fseek(huff_fp, 0, SEEK_END);
		fprintf(huff_fp, "%c", t->ascii_code);
		fprintf(huff_fp, "%c", bit_length);
		unsigned char codeword = NULL;
		int codeword_count = 0;
		for (int j = 0; j < (bit_length - 1) / 8 + 1; j++) {
			codeword = NULL;
			for (int i = 0; i < 8; i++) {
				if (temp_c[codeword_count] == NULL) break;	//temp_c는 개당 8bit
				if (temp_c[codeword_count] & 0x01 == 0x01) {
					codeword += 0x01 << (7 - i);		//이를 concatenate
				}
				else {
					codeword += 0x00 << (7 - i);
				}
				codeword_count++;	//배열 ++
			}
			fprintf(huff_fp, "%c", codeword);	//8개마다 출력
		}
	}
	fclose(huff_fp);
	tnum += bit_length % 8;
	while (1) {
		if (tnum >= 8) tnum -= 8;
		else break;
	}
	return tnum;
}


// Normal Huffman Tree -> In-order Traversal
void Inorder_traversal(HuffmanTreeNode* t, string num, int &tnum) {
	if (t != NULL) {
		Inorder_traversal(t->leftnode, num + "0", tnum);
		if (t->ascii_code != NULL) {
			t->encoded = num;
			tnum = MakeTable_Normal(t, tnum);
		}
		Inorder_traversal(t->rightnode, num + "1", tnum);
	}
}


// Create Context Adaptive Huffman Table
int MakeTable_Adaptive(AdaptiveHuffmanTreeNode* t, int &tnum) {
	FILE * huff_fp = fopen("context_adaptive_huffman_table.hbs", "r+b");	//이어쓰기
	if (huff_fp == NULL)	huff_fp = fopen("context_adaptive_huffman_table.hbs", "a+b");
	unsigned char bit_length = t->encoded.length();
	unsigned char * temp_c;
	temp_c = new unsigned char[bit_length + 1];
	strcpy((char *)temp_c, t->encoded.c_str());
	if (tnum != 0) {
		unsigned char preceding_hi = t->preceding_symbol >> tnum;	//preceding symbol 자름
		unsigned char preceding_lo = t->preceding_symbol << (8 - tnum);
		unsigned char ascii_hi = t->ascii_code >> tnum;
		unsigned char ascii_lo = t->ascii_code << (8 - tnum);	//ascii 자름
		unsigned char bit_length_hi = bit_length >> tnum;
		unsigned char bit_length_lo = bit_length << (8 - tnum);	//bitlength 자름
		fseek(huff_fp, 0, SEEK_END);
		fseek(huff_fp, -1L, SEEK_CUR);
		unsigned char last_char = fgetc(huff_fp);
		unsigned char temp = NULL;
		fseek(huff_fp, -1L, SEEK_CUR);
		temp += last_char;
		temp += preceding_hi;
		fprintf(huff_fp, "%c", temp);
		fprintf(huff_fp, "%c", preceding_lo + ascii_hi);
		fprintf(huff_fp, "%c", ascii_lo + bit_length_hi);
		bool end_flag = false;
		int codeword_count = 0;
		for (int i = 0; i < 8 - tnum; i++) {	//codeword 앞부분
			if (temp_c[codeword_count] == NULL) {
				end_flag = true;
				break;
			}
			if ((temp_c[codeword_count] & 0x01) == 0x01) {	//해당 값이 1인지 판단
				bit_length_lo += 0x01 << (7 - tnum - i);
			}
			else {
				bit_length_lo += 0x00 << (7 - tnum - i);
			}
			codeword_count++;
		}
		fprintf(huff_fp, "%c", bit_length_lo);
		int temp_cc = codeword_count;
		if (end_flag == false) {
			unsigned char codeword_lo = NULL;
			for (int j = 0; j < (bit_length - temp_cc - 1) / 8 + 1; j++) {	//codeword뒷부분 + stuffing bit
				codeword_lo = NULL;
				if (temp_c[codeword_count] == NULL) break;
				for (int i = 0; i < 8; i++) {
					if (temp_c[codeword_count] == NULL) break;
					if ((temp_c[codeword_count] & 0x01) == 0x01) {	//해당값이 1인지 판단하여
						codeword_lo += 0x01 << (7 - i);
					}
					else {
						codeword_lo += 0x00 << (7 - i);
					}
					codeword_count++;
				}
				fprintf(huff_fp, "%c", codeword_lo);	//저장
			}
		}
	}
	else {
		fseek(huff_fp, 0, SEEK_END);
		fprintf(huff_fp, "%c", t->preceding_symbol);
		fprintf(huff_fp, "%c", t->ascii_code);
		fprintf(huff_fp, "%c", bit_length);
		unsigned char codeword = NULL;
		int codeword_count = 0;
		for (int j = 0; j < (bit_length - 1) / 8 + 1; j++) {
			codeword = NULL;
			for (int i = 0; i < 8; i++) {
				if (temp_c[codeword_count] == NULL) break;	//temp_c는 개당 8bit
				if ((temp_c[codeword_count] & 0x01) == 0x01) {
					codeword += 0x01 << (7 - i);		//이를 concatenate
				}
				else {
					codeword += 0x00 << (7 - i);
				}
				codeword_count++;	//배열 ++
			}
			fprintf(huff_fp, "%c", codeword);	//8개마다 출력
		}
	}
	fclose(huff_fp);
	tnum += bit_length % 8;
	while (1) {
		if (tnum >= 8) tnum -= 8;
		else break;
	}
	return tnum;
}


// Adaptive Tree -> In-order Traversal
void A_Inorder_traversal(AdaptiveHuffmanTreeNode* t, string num, int &tnum) {
	if (t != NULL) {
		A_Inorder_traversal(t->leftnode, num + "0", tnum);	//왼쪽 + 0
		if (t->ascii_code != NULL) {
			if (num == "") num = "0";	//만약 N/A라면 0으로 인코딩
			t->encoded = num;
			tnum = MakeTable_Adaptive(t, tnum);
		}
		A_Inorder_traversal(t->rightnode, num + "1", tnum);	//오른쪽 + 0
	}
}


// Encoding
void Encoding(unsigned char * codeword, int arr_size, int &tnum, unsigned char length, int fname) {
	FILE * code_fp;
	if (fname == 0) {	//training_input
		code_fp = fopen("training_input_code.hbs", "r+b");	//결과
		if (code_fp == NULL) code_fp = fopen("training_input_code.hbs", "a+b");
	}
	else if (fname == 1) {	//test1_input
		code_fp = fopen("test_input1_code.hbs", "r+b");	//결과
		if (code_fp == NULL) code_fp = fopen("test_input1_code.hbs", "a+b");
	}
	else if (fname == 2) {	//test2_input
		code_fp = fopen("test_input2_code.hbs", "r+b");	//결과
		if (code_fp == NULL) code_fp = fopen("test_input2_code.hbs", "a+b");
	}
	else {	//test3_input
		code_fp = fopen("test_input3_code.hbs", "r+b");	//결과
		if (code_fp == NULL) code_fp = fopen("test_input3_code.hbs", "a+b");
	}

	if (tnum == 0) {
		fseek(code_fp, 0, SEEK_END);
		for (int i = 0; i < arr_size; i++) {
			fprintf(code_fp, "%c", codeword[i]);
		}
	}
	else {
		unsigned char temp_ov = NULL;
		bool ov_flag = false;
		if (arr_size == 1) {	//bit 가 짧은 경우
			if (length + tnum > 8) {	//bit loss가 발생하는 경우
				temp_ov = codeword[0];
				temp_ov = temp_ov >> 8 - length;
				temp_ov = temp_ov << 16 - length - tnum;	//loss bit temporary
				ov_flag = true;
			}
		}
		else {	//bit가 긴 경우
			for (int i = arr_size - 1; i > 0; i--) {
				if (i == arr_size - 1) {	//가장 하위 비트의 경우
					if (length + tnum > 8) {	//shift bit loss가 발생할 경우
						temp_ov = codeword[i];
						temp_ov = temp_ov >> 8 - length;
						temp_ov = temp_ov << 16 - length - tnum;	//loss bit temporary
						ov_flag = true;
					}
				}
				codeword[i] = codeword[i] >> tnum;
				codeword[i] += codeword[i - 1] << 8 - tnum;
			}
		}
		codeword[0] = codeword[0] >> tnum;		//codeword재정비

		fseek(code_fp, 0, SEEK_END);
		fseek(code_fp, -1L, SEEK_CUR);
		unsigned char result = fgetc(code_fp);
		fseek(code_fp, -1L, SEEK_CUR);
		result += codeword[0];
		fprintf(code_fp, "%c", result);
		for (int i = 1; i < arr_size; i++) {	//나머지 출력
			fprintf(code_fp, "%c", codeword[i]);
		}
		if (ov_flag == true) {
			fprintf(code_fp, "%c", temp_ov);
		}
	}
	tnum += length;
	if (tnum >= 8)	tnum -= 8;
	fclose(code_fp);
}


// Get From Normal Huffman Table
void GetFromTable(unsigned char ch, int &wr_tnum, int fname) {
	FILE * table_fp = fopen("huffman_table.hbs", "rb");	//허프만 테이블

	int tnum = 0;	//끊는 위치
	unsigned char ascii_table, ascii_table_hi, ascii_table_lo = NULL;	//ascii
	unsigned char bl_table, bl_table_hi, bl_table_lo = NULL;	//bit length
	unsigned char * codeword;	//encoded code

	while (1) {
		ascii_table_hi = fgetc(table_fp) << tnum;		// N = table_fp 위치
		ascii_table_lo = fgetc(table_fp) >> (8 - tnum);	// N+1
		if (feof(table_fp)) {	//if N+1 == null -> EOF
			break;
		}
		ascii_table = ascii_table_hi + ascii_table_lo;	//읽어온 table에서 ascii 값 결정
		fseek(table_fp, -1L, SEEK_CUR);		// N

		bl_table_hi = fgetc(table_fp) << tnum; // N+1
		bl_table_lo = fgetc(table_fp) >> (8 - tnum);	// N+2
		bl_table = bl_table_hi + bl_table_lo;	//bit length값 결정
		fseek(table_fp, -1L, SEEK_CUR);		// N+1

		int arr_size = (bl_table - 1) / 8 + 1;
		codeword = new unsigned char[arr_size];
		memset(codeword, 0, arr_size * sizeof(unsigned char));
		unsigned char last_bit_length = NULL;
		if (ascii_table == ch) {	//찾던 결과
			int code_cnt = bl_table;
			for (int i = 0; i < arr_size; i++) {	//get codeword
				unsigned char temp_cw = NULL;
				if (tnum + code_cnt > 8) {		//bit 가 삐져나간경우
					temp_cw = fgetc(table_fp) << tnum;	//앞부분
					codeword[i] += temp_cw;
					temp_cw = fgetc(table_fp) >> (8 - tnum);	//뒷부분
					code_cnt -= 8 - tnum;
					if (tnum - code_cnt < 0) {	//뒤에 더 남은경우
						codeword[i] += temp_cw;
						fseek(table_fp, -1L, SEEK_CUR);
						code_cnt -= tnum;
					}
					else {	//뒤에 안남은경우
						last_bit_length = (8 - tnum) + code_cnt;
						unsigned char shamt = 8 - last_bit_length;
						temp_cw = (temp_cw & (0xff << shamt));	//다 왼쪽으로 채움
						codeword[i] += temp_cw;
					}
				}
				else {	//안삐져나간경우
					temp_cw = fgetc(table_fp) << tnum;
					temp_cw = (temp_cw & (0xff << (8 - code_cnt)));	//다 왼쪽으로 채움
					codeword[i] += temp_cw;
					last_bit_length = code_cnt;
				}
			}
			Encoding(codeword, arr_size, wr_tnum, last_bit_length, fname);	//파일 출력
			break;
		}
		else {	//아닌결과
			tnum += bl_table;
			while (1) {
				if (tnum >= 8) {
					fseek(table_fp, 1L, SEEK_CUR);		// N+1
					tnum -= 8;
				}
				else break;
			}
			continue;
		}
	}
	fclose(table_fp);
}


// Get From Context Adaptive Huffman Table
int GetFromAdaptiveTable(unsigned char prev, unsigned char ch, int &wr_tnum, int fname) {
	int return_val = 0;
	FILE * table_fp = fopen("context_adaptive_huffman_table.hbs", "rb");
	if (table_fp == NULL) {
		return return_val;
	}
	int tnum = 0;
	unsigned char preceding_hi, preceding_lo, preceding = NULL;
	unsigned char ascii_table, ascii_hi, ascii_lo = NULL;
	unsigned char bl_table, bl_hi, bl_lo;
	unsigned char * codeword; //encoded code

	while (1) {
		preceding_hi = fgetc(table_fp) << tnum;	//N = table_fp 위치
		preceding_lo = fgetc(table_fp) >> (8 - tnum);	//N+1
		if (feof(table_fp)) {	//if N+1 == null EOF
			break;
		}
		preceding = preceding_hi + preceding_lo;	//preceding symbol 가져옴
		fseek(table_fp, -1L, SEEK_CUR);		// N

		ascii_hi = fgetc(table_fp) << tnum;	//N+1
		ascii_lo = fgetc(table_fp) >> (8 - tnum);	//N+2
		ascii_table = ascii_hi + ascii_lo;	//current ascii구해옴
		fseek(table_fp, -1L, SEEK_CUR);		// N+1

		bl_hi = fgetc(table_fp) << tnum;	// N+2
		bl_lo = fgetc(table_fp) >> (8 - tnum);	// N+3
		bl_table = bl_hi + bl_lo;	//bit length 구해옴
		fseek(table_fp, -1L, SEEK_CUR);		// N+2

		int arr_size = (bl_table - 1) / 8 + 1;
		unsigned char last_bit_length = NULL;
		if (prev == preceding && ch == ascii_table) {	//찾던 결과
			codeword = new unsigned char[arr_size];
			memset(codeword, 0, arr_size * sizeof(unsigned char));
			return_val = 1;	//Adaptive Table에서 인코딩을 했다는 뜻.
			int code_cnt = bl_table;
			for (int i = 0; i < arr_size; i++) {	//인코딩 된 코드 가져옴
				unsigned char temp_cw = NULL;
				if (tnum + code_cnt > 8) {	//bit가 삐져나간경우
					temp_cw = fgetc(table_fp) << tnum;
					codeword[i] += temp_cw;
					temp_cw = fgetc(table_fp) >> (8 - tnum);
					code_cnt -= 8 - tnum;
					if (tnum - code_cnt < 0) {	//뒤에 더 남은경우
						codeword[i] += temp_cw;
						fseek(table_fp, -1L, SEEK_CUR);
						code_cnt -= tnum;
					}
					else {	//뒤에 안남은경우
						last_bit_length = (8 - tnum) + code_cnt;
						unsigned char shamt = 8 - last_bit_length;
						temp_cw = (temp_cw & (0xff << shamt)); //왼쪽으로 밀어 채움
						codeword[i] += temp_cw;
					}
				}
				else {
					temp_cw = fgetc(table_fp) << tnum;
					temp_cw = (temp_cw & (0xff << (8 - code_cnt)));	//다 왼쪽으로 채움
					codeword[i] += temp_cw;
					last_bit_length = code_cnt;
				}
			}
			Encoding(codeword, arr_size, wr_tnum, last_bit_length, fname); //파일 출력
			break;
		}
		else {	//아닌 결과
			tnum += bl_table;
			while (1) {
				if (tnum >= 8) {	//file pointer, tnum조정
					fseek(table_fp, 1L, SEEK_CUR);	//N+1
					tnum -= 8;
				}
				else break;
			}
			continue;
		}
	}
	fclose(table_fp);
	return return_val;	//사용했으면 1 아니면 0 반환
}


// Control Using Normal/Adaptive
void TableControl(unsigned char prev, unsigned char cur, int &wr_tnum, int fname) {

	if (prev == 0) {
		GetFromTable(cur, wr_tnum, fname);	// first character
	}
	else {
		if (GetFromAdaptiveTable(prev, cur, wr_tnum, fname) == 0) {	//no adaptive table
			GetFromTable(cur, wr_tnum, fname);	//use normal table
		}
	}
}


// main
int main() {
	// ============ Step 2. Step 1. Load Training File ============ // 
	// Step 1.1 Load Training File
	FILE * input = fopen("training_input.txt", "r");
	if (!input) {
		printf("file not exist\n");
		return 0;
	}
	unsigned char chTemp;	// x
	unsigned char chPrev = 0;	// x-1
	int ascii[128] = { 0, };
	int ascii_adaptive[128][128] = { 0, };
	unsigned char end_of_data = 0x7f;
	while (1) {
		chTemp = fgetc(input);	//read current character
		if (feof(input)) {	//file end -> break
			ascii_adaptive[chPrev][end_of_data]++;
			break;
		}
		ascii[chTemp]++;	//count up - normal huffman
		if (chPrev != NULL)
			ascii_adaptive[chPrev][chTemp]++;	//count up - adaptive huffman
		chPrev = chTemp;
	}
	// Step 1.2 End of Data 설정
	ascii[end_of_data]++;


	// ============ Step 2. Normal Huffman Table ============ // 
	// Step 2.1 Create Min-Heap		(using priority_queue)
	priority_queue< pair<int, HuffmanTreeNode*>, vector< pair<int, HuffmanTreeNode*> >, greater< pair<int, HuffmanTreeNode*> > > pq;
	for (int i = 0; i < 128; i++) {
		if (ascii[i] >= 0) {
			HuffmanTreeNode *t;
			t = new HuffmanTreeNode;	//노드 설정
			t->ascii_code = i;
			t->freq = ascii[i];
			t->leftnode = NULL;
			t->rightnode = NULL;
			pq.push(make_pair(ascii[i], t));
		}
	}
	// Step 2.2 Insert Node to Normal Huffman Tree
	HuffmanTreeNode* root;
	while (1) {
		pair<int, HuffmanTreeNode*> temp;
		if (pq.size() == 1) {
			temp = pq.top();
			root = temp.second;
			break;
		}
		temp = pq.top();
		pq.pop();
		HuffmanTreeNode *u, *v, *p;
		u = new HuffmanTreeNode;	//u노드 설정
		u->ascii_code = temp.second->ascii_code;
		u->freq = temp.first;
		u->leftnode = temp.second->leftnode;
		u->rightnode = temp.second->rightnode;

		temp = pq.top();
		pq.pop();
		v = new HuffmanTreeNode;	//v노드 설정
		v->ascii_code = temp.second->ascii_code;
		v->freq = temp.first;
		v->leftnode = temp.second->leftnode;
		v->rightnode = temp.second->rightnode;


		p = HuffmanTree_Insert(u, v);
		pq.push(make_pair(p->freq, p));
	}
	// Step 2.3 Make Normal Huffman Table
	string encoded_num;
	int tnum = 0;
	Inorder_traversal(root, encoded_num, tnum);


	// ============ Step 3. Create Context Adaptive Huffman Table ============ // 
	int atnum = 0;
	for (int i = 0; i < 128; i++) {
		// Step 3.1 Create Min-Heap of each preceding symbol (X-1)
		priority_queue< pair<int, AdaptiveHuffmanTreeNode*>, vector< pair<int, AdaptiveHuffmanTreeNode*> >, greater< pair<int, AdaptiveHuffmanTreeNode*> > > apq;
		for (int j = 0; j < 128; j++) {	//노드 설정
			if (ascii[i] > 600) {	// parameter for minimal cost (normal huffman frequency)
				AdaptiveHuffmanTreeNode * at;
				at = new AdaptiveHuffmanTreeNode;
				at->preceding_symbol = i;		//이전 문자
				at->ascii_code = j;				//현재 문자
				at->freq = ascii_adaptive[i][j];	//빈도수
				at->leftnode = NULL;
				at->rightnode = NULL;
				apq.push(make_pair(ascii_adaptive[i][j], at));	//min-heap
			}
		}
		// Step 3.2 Insert Node to Adaptive Tree
		if (apq.empty()) {
			continue;
		}
		AdaptiveHuffmanTreeNode* aroot;	//조건부 허프만 루트노드
		while (1) {
			pair<int, AdaptiveHuffmanTreeNode*> atemp;
			if (apq.size() == 1) {
				atemp = apq.top();
				aroot = atemp.second;
				break;
			}
			atemp = apq.top();
			apq.pop();
			AdaptiveHuffmanTreeNode *au, *av, *ap;
			au = new AdaptiveHuffmanTreeNode;	//au노드 설정
			au->preceding_symbol = atemp.second->preceding_symbol;
			au->ascii_code = atemp.second->ascii_code;
			au->freq = atemp.first;
			au->leftnode = atemp.second->leftnode;
			au->rightnode = atemp.second->rightnode;

			atemp = apq.top();
			apq.pop();
			av = new AdaptiveHuffmanTreeNode;	//av노드 설정
			av->preceding_symbol = atemp.second->preceding_symbol;
			av->ascii_code = atemp.second->ascii_code;
			av->freq = atemp.first;
			av->leftnode = atemp.second->leftnode;
			av->rightnode = atemp.second->rightnode;

			ap = Adaptive_insert(au, av);
			apq.push(make_pair(ap->freq, ap));
		}
		// Step 3.3 Add encoded number
		string a_encoded_num;
		A_Inorder_traversal(aroot, a_encoded_num, atnum);	//중위순회 하며 context adaptive huffman table . hbs 생성
	}
	fclose(input);

	// ============ Step 4. Encoding - Training Input & Calculate Cost ============ // 
	// Step 4.1 Encoding
	input = fopen("training_input.txt", "r");
	if (!input) {
		printf("file not exist\n");
		return 0;
	}
	tnum = 0;
	chPrev = 0;
	while (1) 
	{
		chTemp = fgetc(input);
		if (feof(input)) {
			TableControl(chPrev, end_of_data, tnum, 0);	//end of data
			break;
		}
		TableControl(chPrev, chTemp, tnum, 0);	//Encoding
		chPrev = chTemp;
	}
	fclose(input);
	// Step 4.2 Calculate Cost
	int data_byte, original_byte, normal_byte, adaptive_byte;
	double cost, C;
	FILE * fp1 = fopen("training_input_code.hbs", "rb");
	if (fp1 != NULL) {
		fseek(fp1, 0, SEEK_END);
		data_byte = ftell(fp1);
	}
	else data_byte = 0;
	FILE * fp2 = fopen("huffman_table.hbs", "rb");
	if (fp2 != NULL) {
		fseek(fp2, 0, SEEK_END);
		normal_byte = ftell(fp2);
	}
	else normal_byte = 0;
	FILE * fp3 = fopen("context_adaptive_huffman_table.hbs", "rb");
	if (fp3 != NULL) {
		fseek(fp3, 0, SEEK_END);
		adaptive_byte = ftell(fp3);
	}
	else adaptive_byte = 0;
	FILE * fp4 = fopen("training_input.txt", "r");
	if (fp4 != NULL) {
		fseek(fp4, 0, SEEK_END);
		original_byte = ftell(fp4);
	}
	else original_byte = 0;
	C = (double)data_byte / (double)original_byte;	//압축 전 / 압축후
	cost = C + (0.0001)*(normal_byte + adaptive_byte);	//cost 구하는 공식
	// print in console
	cout << "data byte: " << data_byte << endl;
	cout << "original byte: " << original_byte << endl;
	cout << "normal byte: " << normal_byte << endl;
	cout << "adaptive byte: " << adaptive_byte << endl;
	cout << "cost: " << cost << endl << endl;
	if(fp1 != NULL)	fclose(fp1);
	if(fp2 != NULL) fclose(fp2);
	if(fp3 != NULL) fclose(fp3);
	if(fp4 != NULL) fclose(fp4);


	// ============ Step 5. Encoding - Test input 1, 2, 3 ============ // 
	// Step 5.1 encoding test1 file
	FILE * fp_test1 = fopen("test_input1.txt", "r");
	if (!fp_test1)
		printf("test_input1 안열림 \n");
	tnum = 0;
	chPrev = 0;
	while (1)
	{
		chTemp = fgetc(input);
		if (feof(input)) {
			TableControl(chPrev, end_of_data, tnum, 1);	//end of data
			break;
		}
		TableControl(chPrev, chTemp, tnum, 1);	//Encoding
		chPrev = chTemp;
	}
	fclose(fp_test1);

	// Step 5.2 encoding test2 file
	FILE * fp_test2 = fopen("test_input2.txt", "r");
	if (!fp_test2)
		printf("test_input2 안열림 \n");
	tnum = 0;
	chPrev = 0;
	while (1)
	{
		chTemp = fgetc(input);
		if (feof(input)) {
			TableControl(chPrev, end_of_data, tnum, 2);	//end of data
			break;
		}
		TableControl(chPrev, chTemp, tnum, 2);	//Encoding
		chPrev = chTemp;
	}
	fclose(fp_test2);

	// Step 5.3 encoding test3 file
	FILE * fp_test3 = fopen("test_input3.txt", "r");
	if (!fp_test3)
		printf("test_input3 안열림 \n");
	tnum = 0;
	chPrev = 0;
	while (1)
	{
		chTemp = fgetc(input);
		if (feof(input)) {
			TableControl(chPrev, end_of_data, tnum, 3);	//end of data
			break;
		}
		TableControl(chPrev, chTemp, tnum, 3);	//Encoding
		chPrev = chTemp;
	}
	fclose(fp_test3);
	return 0;
}