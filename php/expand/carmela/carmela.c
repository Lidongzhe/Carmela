/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_carmela.h"

#ifndef bool
typedef enum __bool { false = 0, true = 1, } bool;
#endif

unsigned char utf8_look_for_table[] = 
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1
};

#define UTFLEN(x)  utf8_look_for_table[(x)]

//计算str字符数目
int getUtf8Length(char *str)
{
	int clen = strlen(str);
	int len = 0;
	char *ptr = str;

	for(; *ptr!=0&&len<clen; len++, ptr+=UTFLEN((unsigned char)*ptr));

	return len;
}

//get子串
char* subUtfString(char *str, unsigned int start, unsigned int end)
{
	int len = getUtf8Length(str);
	int i = 0;

	if(start >= len) return NULL;
	if(end > len) end = len;

	char *sptr = str;
	for(i = 0; i < start; ++i,sptr+=UTFLEN((unsigned char)*sptr));

	char *eptr = sptr;
	for(i = start; i < end; ++i,eptr += UTFLEN((unsigned char)*eptr));

	int retLen = eptr - sptr;
	char *retStr = (char*)malloc(retLen+1);
	memcpy(retStr, sptr, retLen);
	retStr[retLen] = 0;

	return retStr;
}

/***************************************************************************** 
 * 将一个字符的Unicode(UCS-2和UCS-4)编码转换成UTF-8编码. 
 * 
 * 参数: 
 *    unic     字符的Unicode编码值 
 *    pOutput  指向输出的用于存储UTF8编码值的缓冲区的指针 
 *    outsize  pOutput缓冲的大小 
 * 
 * 返回值: 
 *    返回转换后的字符的UTF8编码所占的字节数, 如果出错则返回 0 . 
 * 
 * 注意: 
 *     1. UTF8没有字节序问题, 但是Unicode有字节序要求; 
 *        字节序分为大端(Big Endian)和小端(Little Endian)两种; 
 *        在Intel处理器中采用小端法表示, 在此采用小端法表示. (低地址存低位) 
 *     2. 请保证 pOutput 缓冲区有最少有 6 字节的空间大小! 
 ****************************************************************************/  
int enc_unicode_to_utf8_one(unsigned long unic, unsigned char *pOutput,  
		int outSize)  
{  
	assert(pOutput != NULL);  
	assert(outSize >= 6);  

	if ( unic <= 0x0000007F )  
	{  
		// * U-00000000 - U-0000007F:  0xxxxxxx  
		*pOutput     = (unic & 0x7F);  
		return 1;  
	}  
	else if ( unic >= 0x00000080 && unic <= 0x000007FF )  
	{  
		// * U-00000080 - U-000007FF:  110xxxxx 10xxxxxx  
		*(pOutput+1) = (unic & 0x3F) | 0x80;  
		*pOutput     = ((unic >> 6) & 0x1F) | 0xC0;  
		return 2;  
	}  
	else if ( unic >= 0x00000800 && unic <= 0x0000FFFF )  
	{  
		// * U-00000800 - U-0000FFFF:  1110xxxx 10xxxxxx 10xxxxxx  
		*(pOutput+2) = (unic & 0x3F) | 0x80;  
		*(pOutput+1) = ((unic >>  6) & 0x3F) | 0x80;  
		*pOutput     = ((unic >> 12) & 0x0F) | 0xE0;  
		return 3;  
	}  
	else if ( unic >= 0x00010000 && unic <= 0x001FFFFF )  
	{  
		// * U-00010000 - U-001FFFFF:  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx  
		*(pOutput+3) = (unic & 0x3F) | 0x80;  
		*(pOutput+2) = ((unic >>  6) & 0x3F) | 0x80;  
		*(pOutput+1) = ((unic >> 12) & 0x3F) | 0x80;  
		*pOutput     = ((unic >> 18) & 0x07) | 0xF0;  
		return 4;  
	}  
	else if ( unic >= 0x00200000 && unic <= 0x03FFFFFF )  
	{  
		// * U-00200000 - U-03FFFFFF:  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
		*(pOutput+4) = (unic & 0x3F) | 0x80;  
		*(pOutput+3) = ((unic >>  6) & 0x3F) | 0x80;  
		*(pOutput+2) = ((unic >> 12) & 0x3F) | 0x80;  
		*(pOutput+1) = ((unic >> 18) & 0x3F) | 0x80;  
		*pOutput     = ((unic >> 24) & 0x03) | 0xF8;  
		return 5;  
	}  
	else if ( unic >= 0x04000000 && unic <= 0x7FFFFFFF )  
	{  
		// * U-04000000 - U-7FFFFFFF:  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
		*(pOutput+5) = (unic & 0x3F) | 0x80;  
		*(pOutput+4) = ((unic >>  6) & 0x3F) | 0x80;  
		*(pOutput+3) = ((unic >> 12) & 0x3F) | 0x80;  
		*(pOutput+2) = ((unic >> 18) & 0x3F) | 0x80;  
		*(pOutput+1) = ((unic >> 24) & 0x3F) | 0x80;  
		*pOutput     = ((unic >> 30) & 0x01) | 0xFC;  
		return 6;  
	}  

	return 0;  
}  

/***************************************************************************** 
 * 将一个字符的UTF8编码转换成Unicode(UCS-2和UCS-4)编码. 
 * 
 * 参数: 
 *    pInput      指向输入缓冲区, 以UTF-8编码 
 *    Unic        指向输出缓冲区, 其保存的数据即是Unicode编码值, 
 *                类型为unsigned long . 
 * 
 * 返回值: 
 *    成功则返回该字符的UTF8编码所占用的字节数; 失败则返回0. 
 * 
 * 注意: 
 *     1. UTF8没有字节序问题, 但是Unicode有字节序要求; 
 *        字节序分为大端(Big Endian)和小端(Little Endian)两种; 
 *        在Intel处理器中采用小端法表示, 在此采用小端法表示. (低地址存低位) 
 ****************************************************************************/  
int enc_utf8_to_unicode_one(const unsigned char* pInput, unsigned long *Unic)  
{  
	assert(pInput != NULL && Unic != NULL);  

	// b1 表示UTF-8编码的pInput中的高字节, b2 表示次高字节, ...  
	char b1, b2, b3, b4, b5, b6;  

	*Unic = 0x0; // 把 *Unic 初始化为全零  
	int utfbytes = enc_get_utf8_size(*pInput);  
	unsigned char *pOutput = (unsigned char *) Unic;  

	switch ( utfbytes )  
	{  
		case 0:  
			*pOutput     = *pInput;  
			utfbytes    += 1;  
			break;  
		case 2:  
			b1 = *pInput;  
			b2 = *(pInput + 1);  
			if ( (b2 & 0xC0) != 0x80 )  
				return 0;  
			*pOutput     = (b1 << 6) + (b2 & 0x3F);  
			*(pOutput+1) = (b1 >> 2) & 0x07;  
			break;  
		case 3:  
			b1 = *pInput;  
			b2 = *(pInput + 1);  
			b3 = *(pInput + 2);  
			if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80) )  
				return 0;  
			*pOutput     = (b2 << 6) + (b3 & 0x3F);  
			*(pOutput+1) = (b1 << 4) + ((b2 >> 2) & 0x0F);  
			break;  
		case 4:  
			b1 = *pInput;  
			b2 = *(pInput + 1);  
			b3 = *(pInput + 2);  
			b4 = *(pInput + 3);  
			if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
					|| ((b4 & 0xC0) != 0x80) )  
				return 0;  
			*pOutput     = (b3 << 6) + (b4 & 0x3F);  
			*(pOutput+1) = (b2 << 4) + ((b3 >> 2) & 0x0F);  
			*(pOutput+2) = ((b1 << 2) & 0x1C)  + ((b2 >> 4) & 0x03);  
			break;  
		case 5:  
			b1 = *pInput;  
			b2 = *(pInput + 1);  
			b3 = *(pInput + 2);  
			b4 = *(pInput + 3);  
			b5 = *(pInput + 4);  
			if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
					|| ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80) )  
				return 0;  
			*pOutput     = (b4 << 6) + (b5 & 0x3F);  
			*(pOutput+1) = (b3 << 4) + ((b4 >> 2) & 0x0F);  
			*(pOutput+2) = (b2 << 2) + ((b3 >> 4) & 0x03);  
			*(pOutput+3) = (b1 << 6);  
			break;  
		case 6:  
			b1 = *pInput;  
			b2 = *(pInput + 1);  
			b3 = *(pInput + 2);  
			b4 = *(pInput + 3);  
			b5 = *(pInput + 4);  
			b6 = *(pInput + 5);  
			if ( ((b2 & 0xC0) != 0x80) || ((b3 & 0xC0) != 0x80)  
					|| ((b4 & 0xC0) != 0x80) || ((b5 & 0xC0) != 0x80)  
					|| ((b6 & 0xC0) != 0x80) )  
				return 0;  
			*pOutput     = (b5 << 6) + (b6 & 0x3F);  
			*(pOutput+1) = (b5 << 4) + ((b6 >> 2) & 0x0F);  
			*(pOutput+2) = (b3 << 2) + ((b4 >> 4) & 0x03);  
			*(pOutput+3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);  
			break;  
		default:  
			return 0;  
			break;  
	}  

	return utfbytes;  
}  

/* If you declare any globals in php_carmela.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(carmela)
*/

/* True global resources - no need for thread safety here */
static int le_carmela;

/* {{{ carmela_functions[]
 *
 * Every user visible function must have an entry in carmela_functions[].
 */
const zend_function_entry carmela_functions[] = {
	/*PHP_FE(confirm_carmela_compiled,	NULL)*/		/* For testing, remove later. */
	PHP_FE(carmela_str2ubb,	NULL)
	PHP_FE(carmela_ubb2str,	NULL)
	PHP_FE_END	/* Must be the last line in carmela_functions[] */
};
/* }}} */

/* {{{ carmela_module_entry
 */
zend_module_entry carmela_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"carmela",
	carmela_functions,
	PHP_MINIT(carmela),
	PHP_MSHUTDOWN(carmela),
	PHP_RINIT(carmela),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(carmela),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(carmela),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_CARMELA_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */
#ifdef COMPILE_DL_CARMELA
ZEND_GET_MODULE(carmela)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("carmela.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_carmela_globals, carmela_globals)
    STD_PHP_INI_ENTRY("carmela.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_carmela_globals, carmela_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_carmela_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_carmela_init_globals(zend_carmela_globals *carmela_globals)
{
	carmela_globals->global_value = 0;
	carmela_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(carmela)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(carmela)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(carmela)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(carmela)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(carmela)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "carmela support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_carmela_compiled(string arg)
   Return a string to confirm that the module is compiled in */
/*
PHP_FUNCTION(confirm_carmela_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "carmela", arg);
	RETURN_STRINGL(strg, len, 0);
}
*/
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/

/* {{{ proto string carmela_str2ubb(string str)
    */
PHP_FUNCTION(carmela_str2ubb)
{
	// 声明变量
	char *str = NULL;
	char *result;
	int str_len,result_len=0;
	int argc = ZEND_NUM_ARGS();

	if (zend_parse_parameters(argc TSRMLS_CC, "s", &str, &str_len) == FAILURE) 
		return;
	// 预先计算需要申请内存量
	int i;

	for(i = 0; i < str_len; ++i){
		unsigned char c = str[i];
		// 找到一个宽字符(4+字符不处理)
		if(c >= 239 && c < 247){
			result_len+=15;
			i+=3;
		}else{
			result_len+=1;
		}
	}
	// 没有找到第一平面字符，直接退出
	if(result_len == str_len){
		RETURN_STRINGL(str, str_len, 0);
	}

	// 预先申请+1空间
	result = (char *) emalloc(result_len + 1);
	// 进行替换操作
	int j = 0;
	unsigned char word[5];
	unsigned char new_word[9];
	unsigned char u_word[16];

	for(i = 0; i<str_len; ++i){
		unsigned char c = str[i];
		// 找到一个宽字符
		if(c >= 239 && c < 247){
			// 找到一个宽字符
			char *str_ptr = str;
			str_ptr += i;
			memset(word,0,5);
			memset(new_word,0,9);
			memset(u_word,0,16);
			// 进行组装替换（找到四个字节）
			memcpy(word, str_ptr, 4);
			int k = 0;
			for(; k < 4; ++k){
				sprintf(new_word+(k*2),"%x",(unsigned char)(word[k]));
			}
			int len = sprintf(&u_word, "[u]%s[/u]", new_word);
			char *result_ptr = result;
			result_ptr += j;
			memcpy(result_ptr,u_word,len);
			j += len;
			// 跳过4个字符
			i += 3;
		}else{
			result[j++] = str[i];
		}
	}
	result[result_len] = '\0';

	RETURN_STRINGL(result, result_len, 0);
}
/* }}} */

/* {{{ proto string carmela_ubb2str(string str)
    */
PHP_FUNCTION(carmela_ubb2str)
{
	char *str = NULL;
	int argc = ZEND_NUM_ARGS();
	int str_len;
	int i;
	bool is_find;

	if (zend_parse_parameters(argc TSRMLS_CC, "s", &str, &str_len) == FAILURE) 
		return;
	// 查找[u] 或者 [emoji] 字符
	is_find = false;
	for(i = 0; i < str_len; ++i){
		if('[' == str[i]){
			// 找到第一个字符集合
			char *ptr = str;
			if(memcmp(ptr+i,"[u]",3)){
				// 找到字符串
				is_find = true;
				break;
			}
		}
	}
	// 没有找到相关字符串,直接退出
	if(!is_find){
		RETURN_STRINGL(str, str_len, 0);
	}

	char *result;
	// 预先申请+1空间
	result = (char *) emalloc(str_len + 1);

	int j = 0;
	for(i = 0; i<str_len; ++i){
		if(str[i] == '['){
			// 找到第一个字符集合
			char *ptr = str;
			// 找到[u字符]
			if(memcmp(ptr+i,"[u]",3) == 0){
				// 查找结束字符	
				int k = i+1;
				is_find = false;
				for(;k<str_len;++k){
					//printf("----k:%d\n",k);
					 // 找到结束字符
					 if((k+4 < str_len) && '[' == str[k]){
						if(memcmp(ptr+k,"[/u]",4) == 0){
							is_find = true;
							break;
						}
					}
				}
				if(is_find){
					// 字符串拷贝
					int emoji_len = k-i-3;
					char emoji[emoji_len+1];
					memset(emoji, 0, emoji_len+1);
					memcpy(emoji,ptr+i+3,emoji_len);
					int x = 0;
					// 查找字符串
					char temp[3];
					memset(temp,0,3);
					for(;x<emoji_len;x+=2){
						memcpy(temp,ptr+i+3+x,2);
						result[j++] = (unsigned char)strtol(temp,NULL,16);	
					}
					i=k+3;	
				}
			}
		}else{
			result[j++] = str[i];
		}
	}
	result[j] = '\0';

	RETURN_STRINGL(result, j, 0);


}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
