#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "workflow/HttpMessage.h"
#include "workflow/HttpUtil.h"
#include "workflow/WFServer.h"
#include "workflow/WFHttpServer.h"
#include "workflow/WFFacilities.h"
#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>
#include <lexbor/core/fs.h>
#include <lexbor/dom/dom.h>
#include "base.h"
#include<iostream>
#include<jsoncpp/json/json.h>
#include<unistd.h>

static std::string html_md5_ori("");
static std::string json_output("");

std::string collection_elements_to_json(lxb_dom_collection_t *collection)
{
    std::string json_element_result("");
    lxb_dom_element_t *element;
    Json::Value array_of_json;
    const lxb_char_t *content;
    size_t content_len;
    const lxb_char_t name[] = "href";
    size_t name_size = sizeof(name) - 1;;
    size_t value_len;

    for (size_t i = 0; i < lxb_dom_collection_length(collection); i++) {
        element = lxb_dom_collection_element(collection, i);

        content = NULL;        
        content = lxb_dom_node_text_content(&(element->node), &content_len);

        if (content == NULL) {
            printf("\nContent is empty\n");
            continue;
        }   
        else {
            PRINT("\nContent: %s", content);             

            /* Get value by qualified name */
            const lxb_char_t* value = lxb_dom_element_get_attribute(element, name, name_size, &value_len);
            if (value == NULL) {
                //Failed to get attribute value by qualified name
                return json_element_result;
            }
 
            char ct_temp[1024]="\0";
            int put = 0;
            const lxb_char_t *content_tmp = content;

            while (*content_tmp != '\0')
            {
                ct_temp[put++] = *content_tmp++;
            }
            std::string ct(ct_temp); 
            std::string first = "curl -s -X POST 'http://baobianapi.pullword.com:9091/get.php' -d'";
            first += ct + "'";
            const char *charCoffee = first.c_str();
            FILE * fp;
            char buffer[80];
            fp=popen(charCoffee,"r");
            fgets(buffer,sizeof(buffer),fp);

            std::string result_json(buffer);
            Json::Reader reader;
            Json::Value resultValue;
            bool parseSuccess = reader.parse(result_json, resultValue, false);
            if (parseSuccess){                            
                if(resultValue["result"].asDouble() < (-0.5)){
                    char href_temp[1024]="\0";
                    put = 0;
                    const lxb_char_t* value_tmp = value;
                    while (*value_tmp != '\0')
                    {
                        href_temp[put++] = *value_tmp++;
                    }

                    std::string hr(href_temp);

                    Json::Value tmp;
                    tmp.append(ct);
                    tmp.append(hr);
                    array_of_json.append(tmp);                  
                }
            }
            pclose(fp);
        }
    }
    Json::FastWriter writer;
    std::string re = writer.write(array_of_json);

    lxb_dom_collection_clean(collection);
	return  re;
}

std::string parse_html()
{
    std::string json_result("");
    lxb_status_t status;
    lxb_html_document_t *document;
    size_t html_len;
    lxb_char_t *html = lexbor_fs_file_easy_read((const lxb_char_t *) "baidu.html", &html_len);    
        
    if (html == NULL || html_len==0) {
        return json_result;
    }

    /* Initialization */
    document = lxb_html_document_create();
    if (document == NULL) {
        PRINT("Failed to create HTML Document");
        lxb_html_document_destroy(document);
        lexbor_free(html);
        return json_result;
    }

    /* Parse */
    status = lxb_html_document_parse(document, html, html_len);
    if (status != LXB_STATUS_OK) {
        PRINT("Failed to parse HTML");
        lxb_html_document_destroy(document);
        lexbor_free(html);

        return json_result;
    }

    if (status != LXB_STATUS_OK) {
        PRINT("Failed to convert HTML to S-Expression");
        lxb_html_document_destroy(document);
        lexbor_free(html);
        return json_result;
    }

    lxb_dom_collection_t *collection_pNew;
    lxb_dom_element_t *body;

    body = lxb_dom_interface_element(document->body);    
    
    collection_pNew = lxb_dom_collection_make(&document->dom_document, 128);
    if (collection_pNew == NULL) {
        return json_result;
    }

        /* Full match */
    status = lxb_dom_elements_by_attr(body, collection_pNew,
                                    (const lxb_char_t *) "id", 2,
                                    (const lxb_char_t *) "pane-news", 9,
                                    true);
    if (status != LXB_STATUS_OK) {
        return json_result;
    }

    lxb_dom_element_t *element_pNew = lxb_dom_collection_element(collection_pNew, 0);

    lxb_dom_collection_t *collection_a = lxb_dom_collection_make(&document->dom_document, 128);
    if (collection_a == NULL) {
        //Failed to create Collection object
        return json_result;
    }    

    status = lxb_dom_elements_by_tag_name(element_pNew,collection_a, (const lxb_char_t *) "a", 1);

        if (status != LXB_STATUS_OK) {
            // Failed to get elements by name
            return json_result;
        }

    json_result = collection_elements_to_json(collection_a);

    lxb_dom_collection_destroy(collection_pNew, true);
    lxb_dom_collection_destroy(collection_a, true);
    lxb_html_document_destroy(document);
    lexbor_free(html);

	return  json_result;
}

void getOrUpadate_html()
{
	char md5_buffer[128];
    FILE * fp_html;
    system("curl -s \"http://news.baidu.com\" > baidu.html");
    fp_html=popen("md5sum baidu.html","r");
    fgets(md5_buffer,sizeof(md5_buffer),fp_html);    
    std::string cal_md5(md5_buffer);
    pclose(fp_html);
        
    if(json_output == ""){
		json_output =  parse_html();
        html_md5_ori = cal_md5;
	}
	else{
		if(html_md5_ori == ""){
			html_md5_ori = cal_md5;
		}

		if(html_md5_ori != cal_md5){
            html_md5_ori = cal_md5;
			json_output =  parse_html();
		}
	}    
}

void process(WFHttpTask *server_task)
{
	protocol::HttpRequest *req = server_task->get_req();
	protocol::HttpResponse *resp = server_task->get_resp();
	long long seq = server_task->get_task_seq();
    
    getOrUpadate_html();
    int add_len  = json_output.size();
	
	resp->append_output_body(json_output.c_str(),add_len);

	/* Set status line if you like. */
	resp->set_http_version("HTTP/1.1");
	resp->set_status_code("200");
	resp->set_reason_phrase("OK");

	resp->add_header_pair("Content-Type", "text/json");
	resp->add_header_pair("Server", "Sogou WFHttpServer");
	if (seq == 9) /* no more than 10 requests on the same connection. */
		resp->add_header_pair("Connection", "close");

	/* print some log */
	char addrstr[128];
	struct sockaddr_storage addr;
	socklen_t l = sizeof addr;
	unsigned short port = 0;

	server_task->get_peer_addr((struct sockaddr *)&addr, &l);
	if (addr.ss_family == AF_INET)
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)&addr;
		inet_ntop(AF_INET, &sin->sin_addr, addrstr, 128);
		port = ntohs(sin->sin_port);
	}
	else if (addr.ss_family == AF_INET6)
	{
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&addr;
		inet_ntop(AF_INET6, &sin6->sin6_addr, addrstr, 128);
		port = ntohs(sin6->sin6_port);
	}
	else
		strcpy(addrstr, "Unknown");

	fprintf(stderr, "Peer address: %s:%d, seq: %lld.\n",
			addrstr, port, seq);    
}

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}

int main(int argc, char *argv[])
{
	unsigned short port;

	signal(SIGINT, sig_handler);

	WFHttpServer server(process);
	port = atoi(argv[1]);
	if (server.start(port) == 0)
	{
		wait_group.wait();
		server.stop();
	}
	else
	{
		perror("Cannot start server");
		exit(1);
	}
    
	return 0;
}