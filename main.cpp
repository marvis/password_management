#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cassert>
#include <iostream>
#include <termios.h>

#include "des.h"
using namespace std;

char * decrypt(char * cipher, int num, string key)
{
	assert(num % 8 == 0);
	char * plain;
	plain = new char[num];
	for(int i = 0; i < num; i++) plain[i] = 0;
	Des_Go(plain, cipher, num, key.c_str(), key.size(), DECRYPT);
	return plain;
}

char * encrypt(char * plain, int num, string key)
{
	assert(num % 8 == 0);
	char * cipher;
	cipher = new char[num];
	for(int i = 0; i < num; i++) cipher[i] = 0;
	Des_Go(cipher, plain, num, key.c_str(), key.size(), ENCRYPT);
	return cipher;
}

string read_cipher_file(string file, string key)
{
	char cipher[10000];
    ifstream ifs(file.c_str(), ios::binary);
	if(ifs.fail())
	{
		cerr<<"cipher file "<<file<<" is not existed"<<endl;
		ifs.close();
		return key;
	}
    ifs.seekg(0, ios::end);
    int n = ifs.tellg();

    ifs.seekg(0, ios::beg);
    ifs.read(cipher, n);
	ifs.close();

	if(n % 8 != 0) n = (n / 8 + 1)*8;

	return decrypt(cipher, n, key);
}

bool save_cipher_file(string plain, string file, string key)
{
	int num = plain.size();
	if(num % 8 != 0)
	{
		num = (num/8 +1)*8;
		plain.resize(num);
	}
	char* cipher = encrypt((char*)plain.c_str(), plain.size(), key);
    ofstream ofs(file.c_str(), ios::binary);
	ofs.write(cipher, num);
	ofs.close();
	cout<<"save cipher success"<<endl;
	return true;
}

bool check_key_valid(string plain, string key)
{
	istringstream iss(plain);
	string key2; iss>>key2;
	return (key == key2);
}

void set_echo(bool enable = true)
{
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if( !enable )
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    (void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

int main(int argc, char * argv[])
{
	if(argc != 2)
	{
		cerr<<"usage : "<<argv[0]<<" <password string> "<<endl;
		cerr<<"   or : "<<argv[0]<<" -v "<<endl;
		return 1;
	}
	string cipher_file;

	// get cipher file name
	if(getenv("PASSWORD_FILE"))
	{
		cipher_file = getenv("PASSWORD_FILE");
	}
	else
	{
		string home_dir = getenv("HOME");
		cipher_file = home_dir + "/.password";
	}

	// input key
	string key;
	cout<<"password : ";
	set_echo(false);
	cin>>key;
	set_echo(true);
	cout<<endl;

	// read file and get plain text
	string plain = read_cipher_file(cipher_file, key);
	if(!check_key_valid(plain, key)) 
	{
		cerr<<"password error"<<endl;
		return 1;
	}

	if(string(argv[1]) == "-v")
	{
		vector<string> lines;
		istringstream iss(plain);
		int i = 1;
		char buf[1000];
		iss.getline(buf, 1000);
		while(iss.good())
		{
			string entry;
			iss>>entry;
			iss.getline(buf,1000);
			lines.push_back(entry + buf);
			cout.width(2);cout.fill('0');
			cout<<i++<<" : "<<entry<<endl;
		}

		cout<<"choose [1] : ";
		cin>>i;
		if(i <= 0 || i > lines.size()) i = 1;
		cout<<lines[i-1]<<endl;
		return 0;
	}
	// add entry
	plain += "\r\n";
	plain += argv[1];

	// save plain text
	save_cipher_file(plain,cipher_file,key);
}
