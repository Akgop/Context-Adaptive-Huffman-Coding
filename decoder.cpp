#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>

using namespace std;


// �Ϲ� ������ Ʈ�� ��� ����ü
typedef struct HuffmanTreeNode {
	string encoded;	//���ڵ� �� ��
	unsigned char ascii_code;	//ASCII CODE
} HuffmanTreeNode;


// Context Adaptive ������ Ʈ�� ��� ����ü
typedef struct AdaptiveHuffmanTreeNode {
	string encoded;
	unsigned char preceding_symbol;
	unsigned char ascii_code;
} AdaptiveHuffmanTreeNode;


// unsigned char �� string���� bit���� �о�� �ٲٴ� �Լ�
void UCharToString(string &result, unsigned char ch, unsigned char length) {
	for (int j = 7; j > 7 - length; j--) {
		if ((0x01 & (ch >> j)) == 0x01) {
			result += '1';
		}
		else {
			result += '0';
		}
	}
}


// HuffmanTreeNode�� ���� �־��ִ� �Լ�
void InsertNormalNode(vector<HuffmanTreeNode*> &v, int size,
	unsigned char * cw, unsigned char lbl, unsigned char ascii) {
	string result;
	HuffmanTreeNode *t;
	t = new HuffmanTreeNode;
	t->ascii_code = ascii;
	for (int i = 0; i < size; i++) {
		if (i == size - 1) {
			UCharToString(result, cw[i], lbl);
		}
		else {
			UCharToString(result, cw[i], 8);
		}
	}
	t->encoded = result;
	v.push_back(t);
}


// Adaptive HuffmanTreeNode�� ���� �־��ִ� �Լ�
void InsertAdaptiveNode(vector<AdaptiveHuffmanTreeNode*> &v, int size,
	unsigned char * cw, unsigned char lbl, unsigned char prev, unsigned char ascii) {
	string result;
	AdaptiveHuffmanTreeNode *t;
	t = new AdaptiveHuffmanTreeNode;
	t->preceding_symbol = prev;
	t->ascii_code = ascii;
	for (int i = 0; i < size; i++) {
		if (i == size - 1) {
			UCharToString(result, cw[i], lbl);
		}
		else {
			UCharToString(result, cw[i], 8);
		}
	}
	t->encoded = result;
	v.push_back(t);
}


// Huffman_table.hbs�� �о� Table(vector)�� ����� �ϴ� �Լ�
void MakeNormalTable(vector<HuffmanTreeNode*> &v) {
	FILE * table_fp = fopen("huffman_table.hbs", "rb");
	if (!table_fp) {
		printf("���� �������� ����\n");
	}
	int tnum = 0;	//���� ��ġ
	unsigned char ascii_table, ascii_table_hi, ascii_table_lo = NULL;	//ascii
	unsigned char bl_table, bl_table_hi, bl_table_lo = NULL;	//bit length
	unsigned char * codeword;	//encoded code
	unsigned char last_bit_length = NULL;

	while (1) {
		ascii_table_hi = fgetc(table_fp) << tnum;		// N = table_fp ��ġ
		ascii_table_lo = fgetc(table_fp) >> (8 - tnum);	// N+1
		if (feof(table_fp)) {	//if N+1 == null -> EOF
			break;
		}
		ascii_table = ascii_table_hi + ascii_table_lo;
		fseek(table_fp, -1L, SEEK_CUR);		// N

		bl_table_hi = fgetc(table_fp) << tnum; // N+1
		bl_table_lo = fgetc(table_fp) >> (8 - tnum);	// N+2
		bl_table = bl_table_hi + bl_table_lo;	//bit length�� ����
		fseek(table_fp, -1L, SEEK_CUR);		// N+1

		int arr_size = (bl_table - 1) / 8 + 1;
		codeword = new unsigned char[arr_size];
		memset(codeword, 0, arr_size * sizeof(unsigned char));
		int code_cnt = bl_table;
		for (int i = 0; i < arr_size; i++) {
			unsigned char temp_cw = NULL;
			if (tnum + code_cnt > 8) {		//bit �� �����������
				temp_cw = fgetc(table_fp) << tnum;	//�պκ�
				codeword[i] += temp_cw;
				temp_cw = fgetc(table_fp) >> (8 - tnum);	//�޺κ�
				code_cnt -= 8 - tnum;
				if (tnum - code_cnt < 0) {	//�ڿ� �� �������
					code_cnt -= tnum;
					codeword[i] += temp_cw;
					fseek(table_fp, -1L, SEEK_CUR);
				}
				else {	//�ڿ� �ȳ������
					last_bit_length = (8 - tnum) + code_cnt;
					unsigned char shamt = 8 - last_bit_length;
					temp_cw = (temp_cw & (0xff << shamt));	//�� �������� ä��
					codeword[i] += temp_cw;
				}
			}
			else {	//�Ȼ��� ������ ���
				temp_cw = fgetc(table_fp) << tnum;
				temp_cw = (temp_cw & (0xff << (8 - code_cnt)));	//�� �������� ä��
				codeword[i] += temp_cw;
				last_bit_length = code_cnt;
			}
		}
		// Huffman struct ����
		InsertNormalNode(v, arr_size, codeword, last_bit_length, ascii_table);

		tnum += bl_table;
		if (tnum % 8 != 0) {
			fseek(table_fp, -1L, SEEK_CUR);		// N -= 1
		}
		while (1) {
			if (tnum >= 8)
				tnum -= 8;
			else break;
		}
	}
	fclose(table_fp);
}


// Context_adaptive_huffman_table.hbs�� �о� Table(vector)�� ����� �ϴ� �Լ�
void MakeAdaptiveTable(vector<AdaptiveHuffmanTreeNode*> &v) {
	FILE * table_fp = fopen("context_adaptive_huffman_table.hbs", "rb");
	if (!table_fp) {
		printf("���� �������� ����\n");
		exit(-1);
	}
	int tnum = 0;
	unsigned char preceding, preceding_hi, preceding_lo = NULL;
	unsigned char ascii_table, ascii_hi, ascii_lo = NULL;
	unsigned char bl_table, bl_hi, bl_lo = NULL;
	unsigned char * codeword;
	unsigned char last_bit_length = NULL;
	while (1) {
		preceding_hi = fgetc(table_fp) << tnum;	//preceding symbol�о��
		preceding_lo = fgetc(table_fp) >> (8 - tnum);
		if (feof(table_fp)) {	//������ ���̸� ��
			break;
		}
		preceding = preceding_hi + preceding_lo;	//preceding symbol
		fseek(table_fp, -1L, SEEK_CUR);		// N	= fp��ġ

		ascii_hi = fgetc(table_fp) << tnum;
		ascii_lo = fgetc(table_fp) >> (8 - tnum);	
		ascii_table = ascii_hi + ascii_lo;			//current symbol
		fseek(table_fp, -1L, SEEK_CUR);		// N+1

		bl_hi = fgetc(table_fp) << tnum;
		bl_lo = fgetc(table_fp) >> (8 - tnum);
		bl_table = bl_hi + bl_lo;					//bit length
		fseek(table_fp, -1L, SEEK_CUR);		// N+2

		int arr_size = (bl_table - 1) / 8 + 1;	//codeword ������ ��
		codeword = new unsigned char[arr_size];
		memset(codeword, 0, arr_size * sizeof(unsigned char));
		int code_cnt = bl_table;
		for (int i = 0; i < arr_size; i++) {
			unsigned char temp_cw = NULL;
			if (tnum + code_cnt > 8) {		//bit �� �����������
				temp_cw = fgetc(table_fp) << tnum;	//�պκ�
				codeword[i] += temp_cw;
				temp_cw = fgetc(table_fp) >> (8 - tnum);	//�޺κ�
				code_cnt -= 8 - tnum;
				if (tnum - code_cnt < 0) {	//�ڿ� �� �������
					code_cnt -= tnum;
					codeword[i] += temp_cw;
					fseek(table_fp, -1L, SEEK_CUR);
				}
				else {	//�ڿ� �ȳ������
					last_bit_length = (8 - tnum) + code_cnt;
					unsigned char shamt = 8 - last_bit_length;
					temp_cw = (temp_cw & (0xff << shamt));	//�� �������� ä��
					codeword[i] += temp_cw;
				}
			}
			else {	//�Ȼ��� ������ ���
				temp_cw = fgetc(table_fp) << tnum;
				temp_cw = (temp_cw & (0xff << (8 - code_cnt)));	//�� �������� ä��
				codeword[i] += temp_cw;
				last_bit_length = code_cnt;
			}
		}
		// Adpative Huffman struct ����
		InsertAdaptiveNode(v, arr_size, codeword, last_bit_length, preceding, ascii_table);

		tnum += bl_table;
		if (tnum % 8 != 0) {
			fseek(table_fp, -1L, SEEK_CUR);		// N -= 1
		}
		while (1) {
			if (tnum >= 8)
				tnum -= 8;
			else break;
		}
	}
	fclose(table_fp);
}


// Normal Table(vector)���� �˻�
unsigned char SearchNormalTable(vector<HuffmanTreeNode*> &v, string s) {
	int cnt = 0;
	while (1) {
		if (v[cnt]->encoded == s) {
			return v[cnt]->ascii_code;
		}
		else {
			if (v.back() == v[cnt]) {
				break;
			}
			cnt++;
		}
	}
	return NULL;
}


// Adaptive Table���� �� ã�� �Լ�
unsigned char SearchAdaptiveTable(vector<AdaptiveHuffmanTreeNode*> &v, unsigned char prev, string s) {
	int cnt = 0;
	while (1) {
		//preceding symbol, ã�°� ch�� ��� �����ϸ�
		if ((prev == v[cnt]->preceding_symbol) && (s == v[cnt]->encoded)) {	//
			return v[cnt]->ascii_code;	//�ش� char�� ��ȯ -> ���ڵ� �Ȱ�
		}
		else {
			if (v.back() == v[cnt]) {
				break;	//������ ��ã����
			}
			cnt++;
		}
	}
	return NULL;	//NULL ��ȯ
}


// ��ȿ�� �˻�
bool IsExist(vector<AdaptiveHuffmanTreeNode*> &v, unsigned char prev) {
	int cnt = 0;
	while (1) {
		if (prev == v[cnt]->preceding_symbol) {	//�ش� �ɺ��� ���� ���̺��� �����ϸ�
			return true;	//true��ȯ
		}
		else {
			if (v.back() == v[cnt]) {
				break;	//������ ��ã����
			}
			cnt++;
		}
	}
	return false;	//���ٴ� false��ȯ
}


// Output Write Function
void WriteASCII(unsigned char ch, int fname) {	//output text ��� ���� ��
	FILE * decoded_fp;
	if (fname == 0) {
		decoded_fp = fopen("training_output.txt", "a+");
		if (!decoded_fp) {
			printf("���� ����\n");
			exit(-1);
		}
	}
	else if (fname == 1) {
		decoded_fp = fopen("test_output1.txt", "a+");
		if (!decoded_fp) {
			printf("���� ����\n");
			exit(-1);
		}
	}
	else if (fname == 2) {
		decoded_fp = fopen("test_output2.txt", "a+");
		if (!decoded_fp) {
			printf("���� ����\n");
			exit(-1);
		}
	}
	else {
		decoded_fp = fopen("test_output3.txt", "a+");
		if (!decoded_fp) {
			printf("���� ����\n");
			exit(-1);
		}
	}
	fprintf(decoded_fp, "%c", ch);
	fclose(decoded_fp);
}


// Normal vs Context Adaptive �������ִ� Control function
void DecodeControl(vector<HuffmanTreeNode*> &nv, vector<AdaptiveHuffmanTreeNode*> &av, unsigned char eod, int fname) {
	int file_size = 0;
	FILE * code_fp;
	if (fname == 0) {
		code_fp = fopen("training_input_code.hbs", "rb");
		if (!code_fp) {
			printf("training_input_code ������ �������� ����\n");
			return;
		}
	}
	else if (fname == 1) {
		code_fp = fopen("test_input1_code.hbs", "rb");
		if (!code_fp) {
			printf("test_input1_code ������ �������� ����\n");
			return;
		}
	}
	else if (fname == 2) {
		code_fp = fopen("test_input2_code.hbs", "rb");
		if (!code_fp) {
			printf("test_input2_code ������ �������� ����\n");
			return;
		}
	}
	else {
		code_fp = fopen("test_input3_code.hbs", "rb");
		if (!code_fp) {
			printf("test_input3_code ������ �������� ����\n");
			return;
		}
	}

	unsigned char code, code_hi, code_lo = NULL;
	unsigned char temp = NULL;
	unsigned char decoded = NULL;
	unsigned char decoded_normal = NULL;
	unsigned char decoded_temp_n = NULL, decoded_temp_a = NULL;
	unsigned char preceding_symbol = NULL;
	int tnum = 0;
	bool find_flag = false;
	bool first_char = true;	//ó�� �д���
	string s, temp_s;
	while (1) {
		code_hi = fgetc(code_fp) << tnum;
		code_lo = fgetc(code_fp) >> (8 - tnum);
		if (feof(code_fp)) {
			code_lo = 0;
		}
		fseek(code_fp, -1L, SEEK_CUR);
		code = code_hi + code_lo;
		for (int i = 7; i >= 0; i--) {
			temp_s = s + "";
			UCharToString(temp_s, code, 8 - i);
			if (first_char == true) {	//�������� �� ó���� ���
				decoded = SearchNormalTable(nv, temp_s);
				first_char = false;
			}
			else {	//�������Ǹ� ó���� �ƴ� ��� normal vs adaptive
				if (IsExist(av, preceding_symbol)) {	//preceding symbol�� ���� ���̺��� �����ϴ°�� ���
					decoded = SearchAdaptiveTable(av, preceding_symbol, temp_s);
				}
				else {	//table�� �������� �ʴ� ��� normal table ���
					decoded = SearchNormalTable(nv, temp_s);
				}
			}
			if (decoded == eod) {	//������ ���� ��� �Լ��� �����Ų��.
				return;
			}
			if (decoded != NULL) {	//���ϴ� ���� ã�� ���
				preceding_symbol = decoded;	//preceding symbol�� �����ϰ�
				s = "";
				temp_s = "";
				find_flag = true;	//flag���� ��� ���󺹱� ��Ų��.
				WriteASCII(decoded, fname);	//���ڵ� �Ѵ�.
				decoded = 0;
				tnum += (8 - i);
				fseek(code_fp, -1L, SEEK_CUR);		// N -= 1
				while (1) {
					if (tnum >= 8) {
						fseek(code_fp, 1L, SEEK_CUR);
						tnum -= 8;
					}
					else break;
				}
				break;
			}
		}
		if (find_flag == false) {	//string�� �̾��.
			s = temp_s;
			temp_s = "";
		}
		find_flag = false;
	}
	fclose(code_fp);
}


int main() {
	// ============ Step 1. Create Table (vector) from hbs files ============ // 
	vector<HuffmanTreeNode*> normal_table;
	MakeNormalTable(normal_table);	//normal table

	vector<AdaptiveHuffmanTreeNode*> adaptive_table;
	MakeAdaptiveTable(adaptive_table);	//adaptive table
	
	int cnt = 0;
	while (1) {
		if (normal_table[cnt] == normal_table.back()) {
			break;
		}
		cout << "'" << normal_table[cnt]->ascii_code
			<< "'\t'" << normal_table[cnt]->encoded << endl;
		cnt++;
	}
	

	// ============ Step 2. Decoding training, test1,2,3 ============ // 
	unsigned char end_of_data = 0x7f;
	int trainig_input = 0;
	int test1 = 1; 
	int test2 = 2;
	int test3 = 3;
	DecodeControl(normal_table, adaptive_table, end_of_data, trainig_input);	//training decode
	DecodeControl(normal_table, adaptive_table, end_of_data, test1);	//test1 decode
	DecodeControl(normal_table, adaptive_table, end_of_data, test2);	//test2 decode
	DecodeControl(normal_table, adaptive_table, end_of_data, test3);	//test3 decode
	return 0;
}
