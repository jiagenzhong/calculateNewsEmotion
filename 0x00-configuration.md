# 配置环境

## 本机环境

* 操作系统：【Ubuntu 20.04】

## 配置workflow

安装【git】、【cmake】、【cmake3】、【openssl-devel】、【gcc-c++】。  

因为本机已经安装了【git】、【gcc】（据我网上检索看到的文章，文章说这个实际包含了【gcc-c++】），在这就不多叙述了。

安装【cmake】、【cmake3】、【openssl-devel】、【gcc-c++】实际使用以下命令：

```
安装【cmake、cmake3】：

sudo apt install cmake

-----------------

安装【openssl-devel】：

sudo apt install openssl
sudo apt install libssl-dev

```

编译【tutorial-01-wget.cc】，测试是否配置成功：
```
g++ -std=c++11  -I /usr/local/sogou/include/  -o tutorial-01-wget  tutorial-01-wget.cc  /usr/local/sogou/lib64/libworkflow.a  -lssl -lpthread -lcrypto
```

能生成【tutorial-01-wget】，则证明配置成功了。

### 参考：
* [ubuntu系统安装openssl-devel](https://www.cnblogs.com/new-journey/p/13323301.html)


----



## 配置lexbor

输入命令：
```
curl -O https://lexbor.com/keys/lexbor_signing.key
apt-key add lexbor_signing.key
```

在【/etc/apt/sources.list.d】这个路径，添加文件【lexbor.list】：
```
  ----------
  Note: 在此路径添加文件，需要利用【su】命令切换到【superuser】模式，不然没权限。
  ----------

echo > lexbor.list
```


编辑【lexbor.list】文件，内容如下：
```
deb https://packages.lexbor.com/ubuntu/ focal liblexbor
deb-src https://packages.lexbor.com/ubuntu/ focal liblexbor
```


输入命令：
```
sudo apt update
sudo apt install liblexbor
sudo apt install liblexbor-dev

```

写个测试函数【test_html.c】，看是否配置成功，内容如下：
```c
#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>


int
main(int argc, const char *argv[])
{
    lxb_status_t status;
    const lxb_char_t *tag_name;
    lxb_html_document_t *document;

    static const lxb_char_t html[] = "<div>Work fine!</div>";
    size_t html_len = sizeof(html) - 1;

    document = lxb_html_document_create();
    if (document == NULL) {
        exit(EXIT_FAILURE);
    }

    status = lxb_html_document_parse(document, html, html_len);
    if (status != LXB_STATUS_OK) {
        exit(EXIT_FAILURE);
    }

    tag_name = lxb_dom_element_qualified_name(lxb_dom_interface_element(document->body), 
                                              NULL);

    printf("Element tag name: %s\n", tag_name);

    lxb_html_document_destroy(document);

    return EXIT_SUCCESS;
}
```

在该文件的目录下，输入命令：
```
gcc test_html.c -llexbor -o test_html
```

能生成【test_html】，则证明配置成功了。


### 参考
* [lexbor](https://lexbor.com/docs/lexbor/)


## 配置JsonCpp library

输入命令：
```
sudo apt-get install libjsoncpp-dev
```


写个文件【profile.json】，内容如下：
```json
{
    "firstname":"AmaNi",
    "lastname":"Signh"
}
```


写个测试函数【test_json.cpp】，看是否配置成功，内容如下：
```cpp
#include<iostream>
#include<fstream>
#include<jsoncpp/json/json.h>

using namespace std;

int main(){
    ifstream ifs("profile.json");
    Json::Reader reader;
    Json::Value obj;

    reader.parse(ifs, obj);
    cout << "Last name: " << obj["lastname"].asCString() <<endl;
    cout << "First name: " << obj["firstname"].asCString() <<endl;
}
```


输入命令：
```
g++ -o test_json test_json.cpp -ljsoncpp
```

生成【test_json】，输入命令：
```
 ./test_json
```

得到如下输出：
```
Last name: Signh
First name: AmaNi
```

这样就证明配置成功了。

### 参考 
* [how-to-install-and-use-json-cpp-library-on-ubuntu-linux-os](https://linux.tips/programming/how-to-install-and-use-json-cpp-library-on-ubuntu-linux-os)
