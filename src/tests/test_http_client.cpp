#include <iostream>
#include "tc_common_new/http_client.h"

using namespace tc;

int main() {

	//http://39.91.109.105:40301/create/new/device?hw_info=S3YLNX0KB75668RWD-WCC4M0FSRNRXAA000000003030000070MDA6MTU6NUQ6QTY6NTA6OEJBMDozNjpCQzozNToxQzo4MA==&platform=windows

	auto client = HttpClient::Make("39.91.109.105", 40301, 
		"/create/new/device?hw_info=S3YLNX0KB75668RWD-WCC4M0FSRNRXAA000000003030000070MDA6MTU6NUQ6QTY6NTA6OEJBMDozNjpCQzozNToxQzo4MA==&platform=windows", 6000);

	auto resp = client->Post();

    std::cout << "status: " << resp.status << std::endl;

    std::cout << "body: " << resp.body << std::endl;

	getchar();

	return 0;
}