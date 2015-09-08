/* COMPILE IT WITH
 * -pipe -Ofast -std=c++11 -pthread
 */

#include <iostream>
#include <string>
#include "c_crypto_geport.hpp"
#include "c_random_generator.hpp"
#include "c_crypto_ed25519.hpp"
#include <list>
#include "sha_src/sha256.hpp"
#include "sha_src/sha512.hpp"

using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::list;

typedef number<cpp_int_backend<512 * 2, 512 * 2, unsigned_magnitude, unchecked, void>> long_type; // TODO
typedef c_crypto_geport<512, 9, sha512<long_type>> c_crypto_geport_def;

string generate_random_string (size_t length) {
	auto generate_random_char = [] () -> char {
			const char Charset[] = "0123456789"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz";
			const size_t max_index = (sizeof(Charset) - 1);
			return Charset[rand() % max_index];
	};
	string str(length, 0);
	generate_n(str.begin(), length, generate_random_char);
	return str;
}

string generate_different_string (const string &msg) {
	string ret(msg);
	auto random = rand() % 10;

	switch (random) {
		case 0:
			ret += ' ';
			break;
		case 1:
			ret.erase(0, 1);
			break;
		case 2:
			if (ret[ret.size() / 2] != 'a')
				ret[ret.size() / 2] = 'a';
			else
				ret[ret.size() / 2] = 'b';
			break;
		case 3:
			ret = ret.substr(0, ret.size() / 2) + ' ' + ret.substr(ret.size() / 2, ret.size() / 2);
			break;
		case 4:
			ret = "";
			break;
		case 5:
			ret = " ";
			break;
		case 6:
			ret = generate_random_string(ret.size());
			break;
		case 7:
			ret = ret.substr(0, ret.size() / 2) + generate_random_string(50) + ret.substr(ret.size() / 2, ret.size() / 2);
			break;
		case 8:
			ret = generate_random_string(500);
			break;
		case 9:
			ret = generate_random_string(2);
			break;
		default:
			break;
	}

	if (msg == ret)
		return generate_different_string(msg);

	return ret;
}

bool check_equality_of_signature (const signed_msg<long_type> &a, const signed_msg<long_type> &b) {
	if (a.pop_count != b.pop_count)
		return false;

	if (a.public_key != b.public_key)
		return false;

	for (size_t i = 0; i < 256 + 8; ++i) {
		if (a.Signature[i] != b.Signature[i])
			return false;
	}

	return true;
}

signed_msg<long_type> generate_different_signature (const signed_msg<long_type> &signature) {
	signed_msg<long_type> ret = signature;
	auto random = rand() % 17;

	switch (random) {
		case 0:
			ret.public_key += (rand() % 10000000L) + 1;
			break;
		case 1:
			ret.Signature[rand() % (256 + 8)] -= (rand() % 10000000L) + 1;
			break;
		case 2:
			ret.public_key += 1;
			break;
		case 3:
			ret.Signature[rand() % (256 + 8)] += 1;
			break;
		case 4:
			ret.pop_count += 1;
			break;
		case 5:
			ret.pop_count = ~ret.pop_count;
			break;
		case 6:
			ret.public_key = ~ret.public_key;
			break;
		case 7:
			ret.pop_count = 256 - ret.pop_count;
			break;
		case 8:
			ret.Signature[0] = ~ret.Signature[0];
			break;
		case 9:
			ret.public_key = ret.pop_count;
			break;
		case 10:
			ret = signed_msg<long_type>();
			break;
		case 11:
			ret.public_key = 0;
			break;
		case 12:
			ret.pop_count = 0;
			break;
		case 13:
			ret.public_key = 0;
			ret.pop_count = 1;
			break;
		case 14:
			ret.public_key = 0;
			ret.pop_count = 0;
			for (auto &v : ret.Signature)
				v = 0;
			break;
		case 15:
			ret.public_key = 0;
			ret.pop_count = 0;
			break;
		case 16:
			ret.public_key = 1;
			ret.pop_count = 1;
			break;
		default:
			break;
	}

	if (check_equality_of_signature(ret, signature))
		return generate_different_signature(signature);

	return ret;
}

void random_generator_test (size_t size) {
	c_random_generator<long_type> generator;
	list<long_type> set;

	for (size_t i = 0; i < size; ++i)
		set.push_back(generator.get_random(33));

	size_t counter = set.size();
	set.unique();
	cout << "there was " << counter - set.size() << " colisions on " << size << " tests" << endl;
}

void print_signed_msg (const signed_msg<long_type> &signature) {
	cout << "  public_key: " << signature.public_key << endl << "  pop count: " << signature.pop_count <<
	endl << "  sig values:" << endl;
	for (auto &in : signature.Signature)
		cout << "    " << in << '\n';
}

void correctness_test (size_t size) {
	size_t jump = 100;
	c_crypto_geport_def geport;
	c_crypto_geport_def::long_type Private_key[256 + 8];
	geport.generate_private_key(Private_key);

	string message, different_message;
	signed_msg<long_type> signature, different_signature;

	for (size_t i = 0; i < size; ++i) {
		message = generate_random_string((size_t)(rand() % 50) + 5);
		signature = geport.sign(message, Private_key);

		if (!geport.verify_sign(message, signature)) {
			cout << "-------------------------------------------------------------\n";
			cout << "veryfing correct message gone wrong" << endl;
			cout << "message: " << message << endl << "signature:\n";
			print_signed_msg(signature);
			return;
		}

		for (size_t counter = 0; counter < 20; ++counter) {
			different_signature = generate_different_signature(signature);
			different_message = generate_different_string(message);

			if (geport.verify_sign(different_message, signature)) {
				cout << "-------------------------------------------------------------\n";
				cout << "veryfing wrong message gone is OK" << endl;
				cout << "message: " << message << endl;
				cout << "wrong message: " << different_message << endl << "signature: \n";
				print_signed_msg(signature);
				return;
			}
			
			if (geport.verify_sign(message, different_signature)) {
				cout << "-------------------------------------------------------------\n";
				cout << "veryfing correct message with wrong signature is OK" << endl;
				cout << "message: " << message << endl << "wrong signature:\n";
				print_signed_msg(signature);
				return;
			}
		}

		if (i % jump == 0) {
			cout << i / jump + 1 << " / " << size / jump << endl;
		}
	}

	cout << "all correctness tests passed!" << endl;
}


int main () {
	ios_base::sync_with_stdio(false);
	random_generator_test(10000);
	correctness_test(10000);
	return 0;
}
