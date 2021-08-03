
/* Code to register embedded modules for meta path based loading if any. */

#include <Python.h>

#include "nuitka/constants_blob.h"

#include "nuitka/unfreezing.h"

/* Type bool */
#ifndef __cplusplus
#include "stdbool.h"
#endif

#if 110 > 0
static unsigned char *bytecode_data[110];
#else
static unsigned char **bytecode_data = NULL;
#endif

/* Table for lookup to find compiled or bytecode modules included in this
 * binary or module, or put along this binary as extension modules. We do
 * our own loading for each of these.
 */
extern PyObject *modulecode___main__(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode__distutils_hack(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode__distutils_hack$override(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode__virtualenv(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$abc(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$base_protocol(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$client(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$client_exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$client_proto(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$client_reqrep(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$client_ws(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$connector(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$cookiejar(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$formdata(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$frozenlist(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$hdrs(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$helpers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$http(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$http_exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$http_parser(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$http_websocket(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$http_writer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$locks(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$log(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$multipart(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$payload(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$payload_streamer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$resolver(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$signals(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$streams(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$tcp_helpers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$tracing(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$typedefs(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$web(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$web_app(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$web_exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$web_fileresponse(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$web_log(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$web_middlewares(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$web_protocol(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$web_request(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$web_response(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$web_routedef(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$web_runner(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$web_server(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$web_urldispatcher(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$web_ws(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aiohttp$worker(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$abc(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands$cluster(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands$generic(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands$geo(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands$hash(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands$hyperloglog(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands$list(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands$pubsub(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands$scripting(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands$server(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands$set(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands$sorted_set(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands$streams(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands$string(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$commands$transaction(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$connection(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$errors(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$locks(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$log(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$parser(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$pool(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$pubsub(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$sentinel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$sentinel$commands(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$sentinel$pool(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$stream(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_aioredis$util(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_async_timeout(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_attr(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_attr$_cmp(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_attr$_compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_attr$_config(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_attr$_funcs(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_attr$_make(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_attr$_next_gen(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_attr$_version_info(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_attr$converters(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_attr$exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_attr$filters(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_attr$setters(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_attr$validators(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_cachetools(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_cachetools$cache(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_cachetools$decorators(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_cachetools$fifo(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_cachetools$keys(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_cachetools$lfu(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_cachetools$lru(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_cachetools$mru(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_cachetools$rr(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_cachetools$ttl(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_certifi(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_certifi$core(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$big5freq(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$big5prober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$chardistribution(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$charsetgroupprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$charsetprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$codingstatemachine(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$cp949prober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$enums(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$escprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$escsm(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$eucjpprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$euckrfreq(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$euckrprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$euctwfreq(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$euctwprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$gb2312freq(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$gb2312prober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$hebrewprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$jisfreq(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$jpcntx(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$langbulgarianmodel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$langgreekmodel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$langhebrewmodel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$langrussianmodel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$langthaimodel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$langturkishmodel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$latin1prober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$mbcharsetprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$mbcsgroupprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$mbcssm(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$sbcharsetprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$sbcsgroupprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$sjisprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$universaldetector(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$utf8prober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_chardet$version(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_charset_normalizer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_charset_normalizer$api(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_charset_normalizer$assets(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_charset_normalizer$cd(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_charset_normalizer$constant(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_charset_normalizer$legacy(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_charset_normalizer$md(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_charset_normalizer$models(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_charset_normalizer$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_charset_normalizer$version(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$_compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$_termui_impl(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$_textwrap(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$_unicodefun(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$_winconsole(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$core(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$decorators(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$formatting(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$globals(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$parser(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$shell_completion(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$termui(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$types(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_click$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_colorama(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_colorama$ansi(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_colorama$ansitowin32(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_colorama$initialise(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_colorama$win32(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_colorama$winterm(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_colorful(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_colorful$ansi(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_colorful$colors(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_colorful$core(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_colorful$styles(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_colorful$terminal(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_colorful$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_contextvars(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_dataclasses(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_filelock(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$_cloud_sdk(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$_default(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$_helpers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$_service_account_info(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$app_engine(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$aws(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$compute_engine(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$compute_engine$_metadata(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$compute_engine$credentials(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$credentials(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$crypt(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$crypt$_cryptography_rsa(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$crypt$_python_rsa(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$crypt$base(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$crypt$es256(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$crypt$rsa(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$environment_vars(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$external_account(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$iam(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$identity_pool(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$impersonated_credentials(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$jwt(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$transport(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$transport$_http_client(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$transport$_mtls_helper(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$transport$requests(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$auth$version(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$oauth2(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$oauth2$_client(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$oauth2$challenges(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$oauth2$credentials(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$oauth2$reauth(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$oauth2$service_account(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$oauth2$sts(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$oauth2$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$descriptor(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$descriptor_database(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$descriptor_pb2(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$descriptor_pool(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$internal(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$internal$api_implementation(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$internal$containers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$internal$decoder(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$internal$encoder(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$internal$enum_type_wrapper(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$internal$extension_dict(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$internal$message_listener(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$internal$python_message(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$internal$type_checkers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$internal$well_known_types(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$internal$wire_format(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$json_format(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$message(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$message_factory(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$pyext(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$pyext$cpp_message(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$reflection(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$symbol_database(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$text_encoding(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_google$protobuf$text_format(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$_auth(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$_channel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$_common(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$_compression(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$_cython(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$_grpcio_metadata(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$_interceptor(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$_plugin_wrapping(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$_runtime_protos(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$_server(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$_simple_stubs(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$_utilities(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$aio(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$aio$_base_call(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$aio$_base_channel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$aio$_base_server(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$aio$_call(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$aio$_channel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$aio$_interceptor(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$aio$_metadata(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$aio$_server(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$aio$_typing(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$aio$_utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$experimental(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_grpc$experimental$aio(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_hiredis(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_hiredis$version(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_idna(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_idna$core(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_idna$idnadata(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_idna$intranges(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_idna$package_data(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_idna$uts46data(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_idna_ssl(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_immutables(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_immutables$_version(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_immutables$map(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$_dask(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$_deprecated_my_exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$_memmapping_reducer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$_multiprocessing_helpers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$_parallel_backends(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$_store_backends(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$backports(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$compressor(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$disk(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$executor(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$cloudpickle(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$cloudpickle$cloudpickle(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$cloudpickle$cloudpickle_fast(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$cloudpickle$compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$_base(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$_posix_reduction(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$_posix_wait(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$_win_reduction(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$_win_wait(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$compat_posix(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$compat_win32(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$context(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$fork_exec(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$managers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$popen_loky_posix(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$popen_loky_win32(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$process(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$queues(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$reduction(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$resource_tracker(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$semlock(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$spawn(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$synchronize(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$backend$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$cloudpickle_wrapper(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$process_executor(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$externals$loky$reusable_executor(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$func_inspect(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$hashing(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$logger(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$memory(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$my_exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$numpy_pickle(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$numpy_pickle_compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$numpy_pickle_utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$parallel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_joblib$pool(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_jsonschema(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_jsonschema$_format(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_jsonschema$_legacy_validators(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_jsonschema$_types(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_jsonschema$_utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_jsonschema$_validators(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_jsonschema$compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_jsonschema$exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_jsonschema$validators(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_lxml(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_multidict(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_multidict$_abc(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_multidict$_compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_multidict$_multidict_py(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$__config__(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$_distributor_init(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$_globals(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$_pytesttester(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$compat$_inspect(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$compat$py3k(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$_add_newdocs(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$_asarray(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$_dtype(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$_dtype_ctypes(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$_exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$_internal(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$_methods(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$_string_helpers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$_type_aliases(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$_ufunc_config(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$arrayprint(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$defchararray(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$einsumfunc(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$fromnumeric(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$function_base(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$getlimits(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$machar(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$memmap(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$multiarray(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$numeric(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$numerictypes(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$overrides(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$records(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$shape_base(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$core$umath(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$ctypeslib(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$dual(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$fft(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$fft$_pocketfft(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$fft$helper(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$_datasource(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$_iotools(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$_version(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$arraypad(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$arraysetops(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$arrayterator(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$financial(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$format(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$function_base(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$histograms(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$index_tricks(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$mixins(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$nanfunctions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$npyio(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$polynomial(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$scimath(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$shape_base(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$stride_tricks(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$twodim_base(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$type_check(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$ufunclike(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$lib$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$linalg(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$linalg$linalg(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$ma(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$ma$core(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$ma$extras(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$ma$mrecords(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$matrixlib(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$matrixlib$defmatrix(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$polynomial(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$polynomial$_polybase(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$polynomial$chebyshev(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$polynomial$hermite(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$polynomial$hermite_e(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$polynomial$laguerre(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$polynomial$legendre(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$polynomial$polynomial(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$polynomial$polyutils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$random(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$random$_pickle(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_numpy$version(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_packaging(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_packaging$__about__(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$build_env(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$cli(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$cli$autocompletion(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$cli$base_command(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$cli$cmdoptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$cli$command_context(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$cli$main(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$cli$main_parser(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$cli$parser(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$cli$progress_bars(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$cli$spinners(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$cli$status_codes(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$commands(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$configuration(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$index(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$index$collector(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$index$package_finder(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$locations(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$models(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$models$candidate(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$models$direct_url(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$models$format_control(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$models$index(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$models$link(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$models$scheme(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$models$search_scope(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$models$selection_prefs(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$models$target_python(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$models$wheel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$network(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$network$auth(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$network$cache(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$network$session(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$network$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$operations(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$operations$build(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$operations$build$metadata(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$operations$build$metadata_legacy(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$operations$freeze(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$operations$install(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$operations$install$editable_legacy(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$operations$install$legacy(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$operations$install$wheel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$pyproject(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$req(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$req$constructors(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$req$req_file(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$req$req_install(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$req$req_set(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$req$req_uninstall(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$appdirs(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$compatibility_tags(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$deprecation(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$direct_url_helpers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$encoding(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$entrypoints(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$filesystem(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$filetypes(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$glibc(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$hashes(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$inject_securetransport(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$logging(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$misc(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$models(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$packaging(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$pkg_resources(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$setuptools_build(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$subprocess(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$temp_dir(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$typing(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$unpacking(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$urls(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$virtualenv(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$utils$wheel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$vcs(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$vcs$bazaar(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$vcs$git(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$vcs$mercurial(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$vcs$subversion(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_internal$vcs$versioncontrol(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$appdirs(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$cachecontrol(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$cachecontrol$adapter(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$cachecontrol$cache(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$cachecontrol$caches(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$cachecontrol$caches$file_cache(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$cachecontrol$caches$redis_cache(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$cachecontrol$compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$cachecontrol$controller(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$cachecontrol$filewrapper(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$cachecontrol$serialize(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$cachecontrol$wrapper(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$certifi(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$certifi$core(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$big5freq(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$big5prober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$chardistribution(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$charsetgroupprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$charsetprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$codingstatemachine(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$cp949prober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$enums(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$escprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$escsm(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$eucjpprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$euckrfreq(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$euckrprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$euctwfreq(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$euctwprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$gb2312freq(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$gb2312prober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$hebrewprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$jisfreq(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$jpcntx(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$langbulgarianmodel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$langgreekmodel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$langhebrewmodel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$langrussianmodel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$langthaimodel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$langturkishmodel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$latin1prober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$mbcharsetprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$mbcsgroupprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$mbcssm(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$sbcharsetprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$sbcsgroupprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$sjisprober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$universaldetector(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$utf8prober(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$chardet$version(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$colorama(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$colorama$ansi(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$colorama$ansitowin32(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$colorama$initialise(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$colorama$win32(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$colorama$winterm(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$contextlib2(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$distlib(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$distlib$_backport(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$distlib$_backport$sysconfig(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$distlib$compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$distlib$resources(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$distlib$scripts(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$distlib$util(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$distro(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$_ihatexml(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$_inputstream(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$_tokenizer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$_trie(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$_trie$_base(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$_trie$py(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$_utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$constants(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$filters(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$filters$alphabeticalattributes(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$filters$base(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$filters$inject_meta_charset(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$filters$optionaltags(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$filters$sanitizer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$filters$whitespace(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$html5parser(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$serializer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$treebuilders(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$treebuilders$base(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$treebuilders$dom(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$treebuilders$etree(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$treebuilders$etree_lxml(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$treewalkers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$treewalkers$base(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$treewalkers$dom(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$treewalkers$etree(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$treewalkers$etree_lxml(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$html5lib$treewalkers$genshi(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$idna(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$idna$core(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$idna$idnadata(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$idna$intranges(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$idna$package_data(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$idna$uts46data(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$msgpack(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$msgpack$_version(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$msgpack$exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$msgpack$ext(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$msgpack$fallback(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$packaging(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$packaging$__about__(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$packaging$_compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$packaging$_structures(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$packaging$_typing(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$packaging$markers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$packaging$requirements(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$packaging$specifiers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$packaging$tags(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$packaging$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$packaging$version(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$pep517(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$pep517$compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$pep517$wrappers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$pkg_resources(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$pkg_resources$py31compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$progress(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$progress$bar(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$progress$spinner(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$pyparsing(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$__version__(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$_internal_utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$adapters(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$api(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$auth(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$certs(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$cookies(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$hooks(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$models(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$packages(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$sessions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$status_codes(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$structures(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$requests$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$retrying(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$six(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$toml(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$toml$decoder(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$toml$encoder(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$toml$tz(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$_collections(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$_version(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$connection(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$connectionpool(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$contrib(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$contrib$_appengine_environ(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$contrib$_securetransport(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$contrib$_securetransport$bindings(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$contrib$_securetransport$low_level(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$contrib$pyopenssl(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$contrib$securetransport(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$contrib$socks(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$fields(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$filepost(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$packages(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$packages$backports(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$packages$backports$makefile(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$packages$six(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$packages$ssl_match_hostname(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$packages$ssl_match_hostname$_implementation(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$poolmanager(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$request(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$response(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$util(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$util$connection(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$util$proxy(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$util$queue(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$util$request(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$util$response(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$util$retry(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$util$ssl_(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$util$ssltransport(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$util$timeout(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$util$url(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$urllib3$util$wait(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$webencodings(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$webencodings$labels(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pip$_vendor$webencodings$x_user_defined(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$asgi(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$context_managers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$decorator(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$exposition(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$gc_collector(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$metrics(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$metrics_core(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$mmap_dict(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$openmetrics(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$openmetrics$exposition(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$platform_collector(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$process_collector(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$registry(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$samples(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_prometheus_client$values(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_psutil(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_psutil$_common(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_psutil$_compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_psutil$_psaix(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_psutil$_psbsd(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_psutil$_pslinux(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_psutil$_psosx(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_psutil$_psposix(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_psutil$_pssunos(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_psutil$_pswindows(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$codec(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$codec$ber(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$codec$ber$decoder(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$codec$ber$encoder(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$codec$ber$eoo(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$codec$cer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$codec$cer$decoder(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$codec$cer$encoder(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$codec$der(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$codec$der$decoder(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$codec$der$encoder(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$compat$binary(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$compat$calling(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$compat$dateandtime(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$compat$integer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$compat$octets(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$compat$string(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$debug(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$error(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$type(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$type$base(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$type$char(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$type$constraint(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$type$error(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$type$namedtype(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$type$namedval(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$type$opentype(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$type$tag(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$type$tagmap(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$type$univ(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1$type$useful(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1_modules(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1_modules$pem(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1_modules$rfc2251(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1_modules$rfc2459(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyasn1_modules$rfc5208(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent$_checked_types(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent$_field_common(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent$_helpers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent$_immutable(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent$_pbag(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent$_pclass(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent$_pdeque(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent$_plist(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent$_pmap(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent$_precord(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent$_pset(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent$_pvector(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent$_toolz(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_pyrsistent$_transformations(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$client_mode_hook(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$function_manager(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$import_thread(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$memory_monitor(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$parameter(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$ray_logging(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$runtime_env(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$services(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$signature(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$thirdparty(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$thirdparty$pathspec(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$thirdparty$pathspec$compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$thirdparty$pathspec$pathspec(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$thirdparty$pathspec$pattern(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$thirdparty$pathspec$patterns(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$thirdparty$pathspec$patterns$gitwildmatch(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$thirdparty$pathspec$util(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$_private$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$actor(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$_kubernetes(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$_kubernetes$config(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$_kubernetes$node_provider(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$autoscaler(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$aws(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$aws$config(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$aws$node_provider(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$aws$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$azure(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$azure$config(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$azure$node_provider(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$cli_logger(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$cluster_dump(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$command_runner(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$commands(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$constants(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$docker(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$event_summarizer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$event_system(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$gcp(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$gcp$config(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$gcp$node_provider(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$legacy_info_string(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$load_metrics(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$local(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$local$config(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$local$coordinator_node_provider(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$local$node_provider(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$log_timer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$node_launcher(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$node_tracker(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$prom_metrics(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$providers(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$resource_demand_scheduler(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$staroid(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$staroid$command_runner(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$staroid$node_provider(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$subprocess_output_util(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$updater(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$_private$util(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$aws(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$azure(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$command_runner(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$gcp(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$kubernetes(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$local(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$node_provider(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$sdk(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$staroid(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$autoscaler$tags(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$client_builder(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$cloudpickle(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$cloudpickle$cloudpickle(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$cloudpickle$cloudpickle_fast(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$cloudpickle$compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$core(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$core$generated(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$core$generated$common_pb2(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$core$generated$gcs_pb2(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$core$generated$gcs_service_pb2(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$core$generated$gcs_service_pb2_grpc(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$core$generated$node_manager_pb2(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$core$generated$node_manager_pb2_grpc(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$core$generated$ray_client_pb2(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$core$generated$ray_client_pb2_grpc(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$cross_language(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$experimental(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$experimental$dynamic_resources(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$experimental$internal_kv(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$experimental$packaging(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$experimental$packaging$load_package(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$external_storage(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$gcs_utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$internal(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$internal$internal_api(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$job_config(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$new_dashboard(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$new_dashboard$actor_utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$new_dashboard$consts(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$new_dashboard$datacenter(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$new_dashboard$memory_utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$new_dashboard$modules(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$new_dashboard$modules$stats_collector(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$new_dashboard$modules$stats_collector$stats_collector_consts(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$new_dashboard$modules$stats_collector$stats_collector_head(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$new_dashboard$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$node(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$profiling(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$ray_constants(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$remote_function(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$resource_spec(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$runtime_context(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$serialization(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$serialization_addons(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$state(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$actor_pool(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$check_serialize(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$api(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$client_pickler(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$common(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$dataclient(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$logsclient(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$options(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$runtime_context(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$server(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$server$dataservicer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$server$logservicer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$server$proxier(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$server$server(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$server$server_pickler(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$server$server_stubs(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client$worker(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$client_connect(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$debug(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$inspect(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$iter(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$iter_metrics(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$placement_group(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$rpdb(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$serialization(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$timer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$tracing(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$util$tracing$tracing_helper(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_ray$worker(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_redis(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_redis$_compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_redis$client(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_redis$connection(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_redis$exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_redis$lock(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_redis$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$__version__(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$_internal_utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$adapters(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$api(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$auth(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$certs(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$compat(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$cookies(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$hooks(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$models(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$packages(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$sessions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$status_codes(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$structures(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_requests$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_rsa(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_rsa$asn1(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_rsa$common(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_rsa$core(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_rsa$key(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_rsa$parallel(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_rsa$pem(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_rsa$pkcs1(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_rsa$prime(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_rsa$randnum(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_rsa$transform(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_six(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_starlette(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_starlette$concurrency(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_starlette$datastructures(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_starlette$types(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_sumo_integration(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_sumo_integration$bridge_helper(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_sumo_integration$carla_simulation(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_sumo_integration$constants(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_sumo_integration$sumo_simulation(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_typing_extensions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$_collections(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$_version(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$connection(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$connectionpool(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$contrib(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$contrib$_appengine_environ(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$contrib$appengine(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$contrib$pyopenssl(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$contrib$socks(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$exceptions(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$fields(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$filepost(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$packages(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$packages$backports(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$packages$backports$makefile(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$packages$six(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$packages$ssl_match_hostname(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$packages$ssl_match_hostname$_implementation(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$poolmanager(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$request(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$response(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$util(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$util$connection(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$util$proxy(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$util$queue(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$util$request(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$util$response(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$util$retry(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$util$ssl_(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$util$ssltransport(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$util$timeout(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$util$url(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_urllib3$util$wait(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$classes(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$classes$constants(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$classes$errors(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$classes$messages(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$classes$perceived_objects(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$classes$sensor_data(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$classes$sensor_loaded_vehicle(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$classes$utils(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$create_sumo_vtypes(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$func(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$my_netconvert_carla(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$netconvert_carla(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$ray(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$ray$messages(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$ray$perceived_objects(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$ray$sensor_data(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_util$sequential_types(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$composer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$constructor(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$cyaml(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$dumper(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$emitter(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$error(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$events(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$loader(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$nodes(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$parser(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$reader(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$representer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$resolver(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$scanner(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$serializer(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yaml$tokens(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yarl(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yarl$_quoting(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yarl$_quoting_py(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_yarl$_url(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);
extern PyObject *modulecode_zipp(PyObject *, struct Nuitka_MetaPathBasedLoaderEntry const *);

static struct Nuitka_MetaPathBasedLoaderEntry meta_path_loader_entries[] = {
    {"__main__", modulecode___main__, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"_distutils_hack", modulecode__distutils_hack, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"_distutils_hack.override", modulecode__distutils_hack$override, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"_virtualenv", modulecode__virtualenv, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp", modulecode_aiohttp, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"aiohttp.abc", modulecode_aiohttp$abc, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.base_protocol", modulecode_aiohttp$base_protocol, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.client", modulecode_aiohttp$client, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.client_exceptions", modulecode_aiohttp$client_exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.client_proto", modulecode_aiohttp$client_proto, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.client_reqrep", modulecode_aiohttp$client_reqrep, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.client_ws", modulecode_aiohttp$client_ws, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.connector", modulecode_aiohttp$connector, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.cookiejar", modulecode_aiohttp$cookiejar, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.formdata", modulecode_aiohttp$formdata, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.frozenlist", modulecode_aiohttp$frozenlist, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.hdrs", modulecode_aiohttp$hdrs, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.helpers", modulecode_aiohttp$helpers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.http", modulecode_aiohttp$http, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.http_exceptions", modulecode_aiohttp$http_exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.http_parser", modulecode_aiohttp$http_parser, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.http_websocket", modulecode_aiohttp$http_websocket, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.http_writer", modulecode_aiohttp$http_writer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.locks", modulecode_aiohttp$locks, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.log", modulecode_aiohttp$log, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.multipart", modulecode_aiohttp$multipart, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.payload", modulecode_aiohttp$payload, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.payload_streamer", modulecode_aiohttp$payload_streamer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.resolver", modulecode_aiohttp$resolver, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.signals", modulecode_aiohttp$signals, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.streams", modulecode_aiohttp$streams, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.tcp_helpers", modulecode_aiohttp$tcp_helpers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.tracing", modulecode_aiohttp$tracing, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.typedefs", modulecode_aiohttp$typedefs, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.web", modulecode_aiohttp$web, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.web_app", modulecode_aiohttp$web_app, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.web_exceptions", modulecode_aiohttp$web_exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.web_fileresponse", modulecode_aiohttp$web_fileresponse, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.web_log", modulecode_aiohttp$web_log, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.web_middlewares", modulecode_aiohttp$web_middlewares, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.web_protocol", modulecode_aiohttp$web_protocol, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.web_request", modulecode_aiohttp$web_request, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.web_response", modulecode_aiohttp$web_response, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.web_routedef", modulecode_aiohttp$web_routedef, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.web_runner", modulecode_aiohttp$web_runner, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.web_server", modulecode_aiohttp$web_server, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.web_urldispatcher", modulecode_aiohttp$web_urldispatcher, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.web_ws", modulecode_aiohttp$web_ws, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aiohttp.worker", modulecode_aiohttp$worker, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis", modulecode_aioredis, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"aioredis.abc", modulecode_aioredis$abc, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.commands", modulecode_aioredis$commands, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"aioredis.commands.cluster", modulecode_aioredis$commands$cluster, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.commands.generic", modulecode_aioredis$commands$generic, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.commands.geo", modulecode_aioredis$commands$geo, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.commands.hash", modulecode_aioredis$commands$hash, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.commands.hyperloglog", modulecode_aioredis$commands$hyperloglog, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.commands.list", modulecode_aioredis$commands$list, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.commands.pubsub", modulecode_aioredis$commands$pubsub, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.commands.scripting", modulecode_aioredis$commands$scripting, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.commands.server", modulecode_aioredis$commands$server, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.commands.set", modulecode_aioredis$commands$set, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.commands.sorted_set", modulecode_aioredis$commands$sorted_set, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.commands.streams", modulecode_aioredis$commands$streams, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.commands.string", modulecode_aioredis$commands$string, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.commands.transaction", modulecode_aioredis$commands$transaction, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.connection", modulecode_aioredis$connection, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.errors", modulecode_aioredis$errors, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.locks", modulecode_aioredis$locks, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.log", modulecode_aioredis$log, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.parser", modulecode_aioredis$parser, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.pool", modulecode_aioredis$pool, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.pubsub", modulecode_aioredis$pubsub, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.sentinel", modulecode_aioredis$sentinel, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"aioredis.sentinel.commands", modulecode_aioredis$sentinel$commands, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.sentinel.pool", modulecode_aioredis$sentinel$pool, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.stream", modulecode_aioredis$stream, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"aioredis.util", modulecode_aioredis$util, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"async_timeout", modulecode_async_timeout, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"attr", modulecode_attr, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"attr._cmp", modulecode_attr$_cmp, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"attr._compat", modulecode_attr$_compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"attr._config", modulecode_attr$_config, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"attr._funcs", modulecode_attr$_funcs, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"attr._make", modulecode_attr$_make, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"attr._next_gen", modulecode_attr$_next_gen, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"attr._version_info", modulecode_attr$_version_info, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"attr.converters", modulecode_attr$converters, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"attr.exceptions", modulecode_attr$exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"attr.filters", modulecode_attr$filters, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"attr.setters", modulecode_attr$setters, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"attr.validators", modulecode_attr$validators, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"cachetools", modulecode_cachetools, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"cachetools.cache", modulecode_cachetools$cache, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"cachetools.decorators", modulecode_cachetools$decorators, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"cachetools.fifo", modulecode_cachetools$fifo, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"cachetools.keys", modulecode_cachetools$keys, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"cachetools.lfu", modulecode_cachetools$lfu, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"cachetools.lru", modulecode_cachetools$lru, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"cachetools.mru", modulecode_cachetools$mru, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"cachetools.rr", modulecode_cachetools$rr, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"cachetools.ttl", modulecode_cachetools$ttl, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"certifi", modulecode_certifi, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"certifi.core", modulecode_certifi$core, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet", modulecode_chardet, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"chardet.big5freq", modulecode_chardet$big5freq, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.big5prober", modulecode_chardet$big5prober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.chardistribution", modulecode_chardet$chardistribution, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.charsetgroupprober", modulecode_chardet$charsetgroupprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.charsetprober", modulecode_chardet$charsetprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.codingstatemachine", modulecode_chardet$codingstatemachine, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.cp949prober", modulecode_chardet$cp949prober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.enums", modulecode_chardet$enums, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.escprober", modulecode_chardet$escprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.escsm", modulecode_chardet$escsm, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.eucjpprober", modulecode_chardet$eucjpprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.euckrfreq", modulecode_chardet$euckrfreq, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.euckrprober", modulecode_chardet$euckrprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.euctwfreq", modulecode_chardet$euctwfreq, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.euctwprober", modulecode_chardet$euctwprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.gb2312freq", modulecode_chardet$gb2312freq, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.gb2312prober", modulecode_chardet$gb2312prober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.hebrewprober", modulecode_chardet$hebrewprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.jisfreq", modulecode_chardet$jisfreq, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.jpcntx", modulecode_chardet$jpcntx, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.langbulgarianmodel", modulecode_chardet$langbulgarianmodel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.langgreekmodel", modulecode_chardet$langgreekmodel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.langhebrewmodel", modulecode_chardet$langhebrewmodel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.langrussianmodel", modulecode_chardet$langrussianmodel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.langthaimodel", modulecode_chardet$langthaimodel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.langturkishmodel", modulecode_chardet$langturkishmodel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.latin1prober", modulecode_chardet$latin1prober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.mbcharsetprober", modulecode_chardet$mbcharsetprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.mbcsgroupprober", modulecode_chardet$mbcsgroupprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.mbcssm", modulecode_chardet$mbcssm, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.sbcharsetprober", modulecode_chardet$sbcharsetprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.sbcsgroupprober", modulecode_chardet$sbcsgroupprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.sjisprober", modulecode_chardet$sjisprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.universaldetector", modulecode_chardet$universaldetector, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.utf8prober", modulecode_chardet$utf8prober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"chardet.version", modulecode_chardet$version, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"charset_normalizer", modulecode_charset_normalizer, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"charset_normalizer.api", modulecode_charset_normalizer$api, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"charset_normalizer.assets", modulecode_charset_normalizer$assets, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"charset_normalizer.cd", modulecode_charset_normalizer$cd, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"charset_normalizer.constant", modulecode_charset_normalizer$constant, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"charset_normalizer.legacy", modulecode_charset_normalizer$legacy, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"charset_normalizer.md", modulecode_charset_normalizer$md, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"charset_normalizer.models", modulecode_charset_normalizer$models, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"charset_normalizer.utils", modulecode_charset_normalizer$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"charset_normalizer.version", modulecode_charset_normalizer$version, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click", modulecode_click, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"click._compat", modulecode_click$_compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click._termui_impl", modulecode_click$_termui_impl, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click._textwrap", modulecode_click$_textwrap, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click._unicodefun", modulecode_click$_unicodefun, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click._winconsole", modulecode_click$_winconsole, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click.core", modulecode_click$core, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click.decorators", modulecode_click$decorators, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click.exceptions", modulecode_click$exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click.formatting", modulecode_click$formatting, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click.globals", modulecode_click$globals, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click.parser", modulecode_click$parser, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click.shell_completion", modulecode_click$shell_completion, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click.termui", modulecode_click$termui, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click.types", modulecode_click$types, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"click.utils", modulecode_click$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"colorama", modulecode_colorama, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"colorama.ansi", modulecode_colorama$ansi, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"colorama.ansitowin32", modulecode_colorama$ansitowin32, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"colorama.initialise", modulecode_colorama$initialise, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"colorama.win32", modulecode_colorama$win32, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"colorama.winterm", modulecode_colorama$winterm, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"colorful", modulecode_colorful, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"colorful.ansi", modulecode_colorful$ansi, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"colorful.colors", modulecode_colorful$colors, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"colorful.core", modulecode_colorful$core, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"colorful.styles", modulecode_colorful$styles, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"colorful.terminal", modulecode_colorful$terminal, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"colorful.utils", modulecode_colorful$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"contextvars", modulecode_contextvars, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"dataclasses", modulecode_dataclasses, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"filelock", modulecode_filelock, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google", modulecode_google, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"google.auth", modulecode_google$auth, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"google.auth._cloud_sdk", modulecode_google$auth$_cloud_sdk, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth._default", modulecode_google$auth$_default, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth._helpers", modulecode_google$auth$_helpers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth._service_account_info", modulecode_google$auth$_service_account_info, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.app_engine", modulecode_google$auth$app_engine, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.aws", modulecode_google$auth$aws, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.compute_engine", modulecode_google$auth$compute_engine, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"google.auth.compute_engine._metadata", modulecode_google$auth$compute_engine$_metadata, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.compute_engine.credentials", modulecode_google$auth$compute_engine$credentials, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.credentials", modulecode_google$auth$credentials, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.crypt", modulecode_google$auth$crypt, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"google.auth.crypt._cryptography_rsa", modulecode_google$auth$crypt$_cryptography_rsa, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.crypt._python_rsa", modulecode_google$auth$crypt$_python_rsa, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.crypt.base", modulecode_google$auth$crypt$base, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.crypt.es256", modulecode_google$auth$crypt$es256, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.crypt.rsa", modulecode_google$auth$crypt$rsa, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.environment_vars", modulecode_google$auth$environment_vars, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.exceptions", modulecode_google$auth$exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.external_account", modulecode_google$auth$external_account, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.iam", modulecode_google$auth$iam, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.identity_pool", modulecode_google$auth$identity_pool, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.impersonated_credentials", modulecode_google$auth$impersonated_credentials, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.jwt", modulecode_google$auth$jwt, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.transport", modulecode_google$auth$transport, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"google.auth.transport._http_client", modulecode_google$auth$transport$_http_client, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.transport._mtls_helper", modulecode_google$auth$transport$_mtls_helper, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.transport.requests", modulecode_google$auth$transport$requests, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.auth.version", modulecode_google$auth$version, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.oauth2", modulecode_google$oauth2, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"google.oauth2._client", modulecode_google$oauth2$_client, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.oauth2.challenges", modulecode_google$oauth2$challenges, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.oauth2.credentials", modulecode_google$oauth2$credentials, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.oauth2.reauth", modulecode_google$oauth2$reauth, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.oauth2.service_account", modulecode_google$oauth2$service_account, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.oauth2.sts", modulecode_google$oauth2$sts, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.oauth2.utils", modulecode_google$oauth2$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf", modulecode_google$protobuf, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"google.protobuf.descriptor", modulecode_google$protobuf$descriptor, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.descriptor_database", modulecode_google$protobuf$descriptor_database, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.descriptor_pb2", modulecode_google$protobuf$descriptor_pb2, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.descriptor_pool", modulecode_google$protobuf$descriptor_pool, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.internal", modulecode_google$protobuf$internal, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"google.protobuf.internal.api_implementation", modulecode_google$protobuf$internal$api_implementation, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.internal.containers", modulecode_google$protobuf$internal$containers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.internal.decoder", modulecode_google$protobuf$internal$decoder, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.internal.encoder", modulecode_google$protobuf$internal$encoder, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.internal.enum_type_wrapper", modulecode_google$protobuf$internal$enum_type_wrapper, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.internal.extension_dict", modulecode_google$protobuf$internal$extension_dict, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.internal.message_listener", modulecode_google$protobuf$internal$message_listener, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.internal.python_message", modulecode_google$protobuf$internal$python_message, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.internal.type_checkers", modulecode_google$protobuf$internal$type_checkers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.internal.well_known_types", modulecode_google$protobuf$internal$well_known_types, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.internal.wire_format", modulecode_google$protobuf$internal$wire_format, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.json_format", modulecode_google$protobuf$json_format, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.message", modulecode_google$protobuf$message, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.message_factory", modulecode_google$protobuf$message_factory, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.pyext", modulecode_google$protobuf$pyext, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"google.protobuf.pyext.cpp_message", modulecode_google$protobuf$pyext$cpp_message, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.reflection", modulecode_google$protobuf$reflection, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.symbol_database", modulecode_google$protobuf$symbol_database, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.text_encoding", modulecode_google$protobuf$text_encoding, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"google.protobuf.text_format", modulecode_google$protobuf$text_format, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc", modulecode_grpc, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"grpc._auth", modulecode_grpc$_auth, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc._channel", modulecode_grpc$_channel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc._common", modulecode_grpc$_common, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc._compression", modulecode_grpc$_compression, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc._cython", modulecode_grpc$_cython, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"grpc._grpcio_metadata", modulecode_grpc$_grpcio_metadata, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc._interceptor", modulecode_grpc$_interceptor, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc._plugin_wrapping", modulecode_grpc$_plugin_wrapping, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc._runtime_protos", modulecode_grpc$_runtime_protos, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc._server", modulecode_grpc$_server, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc._simple_stubs", modulecode_grpc$_simple_stubs, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc._utilities", modulecode_grpc$_utilities, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc.aio", modulecode_grpc$aio, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"grpc.aio._base_call", modulecode_grpc$aio$_base_call, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc.aio._base_channel", modulecode_grpc$aio$_base_channel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc.aio._base_server", modulecode_grpc$aio$_base_server, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc.aio._call", modulecode_grpc$aio$_call, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc.aio._channel", modulecode_grpc$aio$_channel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc.aio._interceptor", modulecode_grpc$aio$_interceptor, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc.aio._metadata", modulecode_grpc$aio$_metadata, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc.aio._server", modulecode_grpc$aio$_server, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc.aio._typing", modulecode_grpc$aio$_typing, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc.aio._utils", modulecode_grpc$aio$_utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"grpc.experimental", modulecode_grpc$experimental, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"grpc.experimental.aio", modulecode_grpc$experimental$aio, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"hiredis", modulecode_hiredis, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"hiredis.version", modulecode_hiredis$version, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"idna", modulecode_idna, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"idna.core", modulecode_idna$core, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"idna.idnadata", modulecode_idna$idnadata, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"idna.intranges", modulecode_idna$intranges, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"idna.package_data", modulecode_idna$package_data, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"idna.uts46data", modulecode_idna$uts46data, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"idna_ssl", modulecode_idna_ssl, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"immutables", modulecode_immutables, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"immutables._version", modulecode_immutables$_version, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"immutables.map", modulecode_immutables$map, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"importlib_metadata", NULL, 0, 22957, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"importlib_metadata._compat", NULL, 1, 4125, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"joblib", modulecode_joblib, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"joblib._dask", modulecode_joblib$_dask, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib._deprecated_my_exceptions", modulecode_joblib$_deprecated_my_exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib._memmapping_reducer", modulecode_joblib$_memmapping_reducer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib._multiprocessing_helpers", modulecode_joblib$_multiprocessing_helpers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib._parallel_backends", modulecode_joblib$_parallel_backends, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib._store_backends", modulecode_joblib$_store_backends, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.backports", modulecode_joblib$backports, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.compressor", modulecode_joblib$compressor, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.disk", modulecode_joblib$disk, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.executor", modulecode_joblib$executor, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals", modulecode_joblib$externals, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"joblib.externals.cloudpickle", modulecode_joblib$externals$cloudpickle, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"joblib.externals.cloudpickle.cloudpickle", modulecode_joblib$externals$cloudpickle$cloudpickle, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.cloudpickle.cloudpickle_fast", modulecode_joblib$externals$cloudpickle$cloudpickle_fast, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.cloudpickle.compat", modulecode_joblib$externals$cloudpickle$compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky", modulecode_joblib$externals$loky, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"joblib.externals.loky._base", modulecode_joblib$externals$loky$_base, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend", modulecode_joblib$externals$loky$backend, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"joblib.externals.loky.backend._posix_reduction", modulecode_joblib$externals$loky$backend$_posix_reduction, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend._posix_wait", modulecode_joblib$externals$loky$backend$_posix_wait, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend._win_reduction", modulecode_joblib$externals$loky$backend$_win_reduction, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend._win_wait", modulecode_joblib$externals$loky$backend$_win_wait, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.compat", modulecode_joblib$externals$loky$backend$compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.compat_posix", modulecode_joblib$externals$loky$backend$compat_posix, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.compat_win32", modulecode_joblib$externals$loky$backend$compat_win32, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.context", modulecode_joblib$externals$loky$backend$context, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.fork_exec", modulecode_joblib$externals$loky$backend$fork_exec, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.managers", modulecode_joblib$externals$loky$backend$managers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.popen_loky_posix", modulecode_joblib$externals$loky$backend$popen_loky_posix, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.popen_loky_win32", modulecode_joblib$externals$loky$backend$popen_loky_win32, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.process", modulecode_joblib$externals$loky$backend$process, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.queues", modulecode_joblib$externals$loky$backend$queues, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.reduction", modulecode_joblib$externals$loky$backend$reduction, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.resource_tracker", modulecode_joblib$externals$loky$backend$resource_tracker, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.semlock", modulecode_joblib$externals$loky$backend$semlock, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.spawn", modulecode_joblib$externals$loky$backend$spawn, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.synchronize", modulecode_joblib$externals$loky$backend$synchronize, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.backend.utils", modulecode_joblib$externals$loky$backend$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.cloudpickle_wrapper", modulecode_joblib$externals$loky$cloudpickle_wrapper, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.process_executor", modulecode_joblib$externals$loky$process_executor, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.externals.loky.reusable_executor", modulecode_joblib$externals$loky$reusable_executor, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.func_inspect", modulecode_joblib$func_inspect, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.hashing", modulecode_joblib$hashing, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.logger", modulecode_joblib$logger, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.memory", modulecode_joblib$memory, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.my_exceptions", modulecode_joblib$my_exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.numpy_pickle", modulecode_joblib$numpy_pickle, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.numpy_pickle_compat", modulecode_joblib$numpy_pickle_compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.numpy_pickle_utils", modulecode_joblib$numpy_pickle_utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.parallel", modulecode_joblib$parallel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"joblib.pool", modulecode_joblib$pool, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"jsonschema", modulecode_jsonschema, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"jsonschema._format", modulecode_jsonschema$_format, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"jsonschema._legacy_validators", modulecode_jsonschema$_legacy_validators, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"jsonschema._types", modulecode_jsonschema$_types, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"jsonschema._utils", modulecode_jsonschema$_utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"jsonschema._validators", modulecode_jsonschema$_validators, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"jsonschema.compat", modulecode_jsonschema$compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"jsonschema.exceptions", modulecode_jsonschema$exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"jsonschema.validators", modulecode_jsonschema$validators, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"lxml", modulecode_lxml, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"multidict", modulecode_multidict, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"multidict._abc", modulecode_multidict$_abc, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"multidict._compat", modulecode_multidict$_compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"multidict._multidict_py", modulecode_multidict$_multidict_py, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy", modulecode_numpy, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.__config__", modulecode_numpy$__config__, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy._distributor_init", modulecode_numpy$_distributor_init, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy._globals", modulecode_numpy$_globals, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy._pytesttester", modulecode_numpy$_pytesttester, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.compat", modulecode_numpy$compat, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.compat._inspect", modulecode_numpy$compat$_inspect, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.compat.py3k", modulecode_numpy$compat$py3k, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core", modulecode_numpy$core, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.core._add_newdocs", modulecode_numpy$core$_add_newdocs, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core._asarray", modulecode_numpy$core$_asarray, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core._dtype", modulecode_numpy$core$_dtype, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core._dtype_ctypes", modulecode_numpy$core$_dtype_ctypes, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core._exceptions", modulecode_numpy$core$_exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core._internal", modulecode_numpy$core$_internal, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core._methods", modulecode_numpy$core$_methods, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core._string_helpers", modulecode_numpy$core$_string_helpers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core._type_aliases", modulecode_numpy$core$_type_aliases, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core._ufunc_config", modulecode_numpy$core$_ufunc_config, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.arrayprint", modulecode_numpy$core$arrayprint, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.defchararray", modulecode_numpy$core$defchararray, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.einsumfunc", modulecode_numpy$core$einsumfunc, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.fromnumeric", modulecode_numpy$core$fromnumeric, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.function_base", modulecode_numpy$core$function_base, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.getlimits", modulecode_numpy$core$getlimits, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.machar", modulecode_numpy$core$machar, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.memmap", modulecode_numpy$core$memmap, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.multiarray", modulecode_numpy$core$multiarray, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.numeric", modulecode_numpy$core$numeric, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.numerictypes", modulecode_numpy$core$numerictypes, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.overrides", modulecode_numpy$core$overrides, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.records", modulecode_numpy$core$records, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.shape_base", modulecode_numpy$core$shape_base, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.core.umath", modulecode_numpy$core$umath, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.ctypeslib", modulecode_numpy$ctypeslib, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.distutils", NULL, 2, 1506, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.distutils.__config__", NULL, 3, 2578, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils._shell_utils", NULL, 4, 3219, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.ccompiler", NULL, 5, 19333, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command", NULL, 6, 1043, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.distutils.command.autodist", NULL, 7, 3830, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.bdist_rpm", NULL, 8, 853, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.build", NULL, 9, 1751, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.build_clib", NULL, 10, 7853, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.build_ext", NULL, 11, 12883, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.build_py", NULL, 12, 1369, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.build_scripts", NULL, 13, 1680, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.build_src", NULL, 14, 18877, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.config", NULL, 15, 13922, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.config_compiler", NULL, 16, 4199, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.develop", NULL, 17, 876, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.egg_info", NULL, 18, 1106, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.install", NULL, 19, 2070, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.install_clib", NULL, 20, 1646, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.install_data", NULL, 21, 920, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.install_headers", NULL, 22, 974, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.command.sdist", NULL, 23, 964, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.conv_template", NULL, 24, 8288, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.core", NULL, 25, 4718, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.cpuinfo", NULL, 26, 33448, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.exec_command", NULL, 27, 9155, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.extension", NULL, 28, 2507, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.fcompiler", NULL, 29, 28461, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.distutils.fcompiler.environment", NULL, 30, 2957, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.from_template", NULL, 31, 7233, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.lib2def", NULL, 32, 3357, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.log", NULL, 33, 2442, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.mingw32ccompiler", NULL, 34, 14325, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.misc_util", NULL, 35, 70428, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.npy_pkg_config", NULL, 36, 12292, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.numpy_distribution", NULL, 37, 791, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.system_info", NULL, 38, 86085, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.distutils.unixccompiler", NULL, 39, 3249, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.dual", modulecode_numpy$dual, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.f2py", NULL, 40, 2662, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.f2py.__version__", NULL, 41, 357, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.f2py.auxfuncs", NULL, 42, 22256, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.f2py.capi_maps", NULL, 43, 18042, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.f2py.cb_rules", NULL, 44, 15441, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.f2py.cfuncs", NULL, 45, 38358, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.f2py.common_rules", NULL, 46, 4830, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.f2py.crackfortran", NULL, 47, 77241, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.f2py.diagnose", NULL, 48, 3713, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.f2py.f2py2e", NULL, 49, 20358, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.f2py.f2py_testing", NULL, 50, 1416, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.f2py.f90mod_rules", NULL, 51, 7339, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.f2py.func2subr", NULL, 52, 6711, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.f2py.rules", NULL, 53, 34080, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.f2py.use_rules", NULL, 54, 3052, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.fft", modulecode_numpy$fft, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.fft._pocketfft", modulecode_numpy$fft$_pocketfft, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.fft.helper", modulecode_numpy$fft$helper, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib", modulecode_numpy$lib, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.lib._datasource", modulecode_numpy$lib$_datasource, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib._iotools", modulecode_numpy$lib$_iotools, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib._version", modulecode_numpy$lib$_version, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.arraypad", modulecode_numpy$lib$arraypad, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.arraysetops", modulecode_numpy$lib$arraysetops, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.arrayterator", modulecode_numpy$lib$arrayterator, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.financial", modulecode_numpy$lib$financial, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.format", modulecode_numpy$lib$format, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.function_base", modulecode_numpy$lib$function_base, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.histograms", modulecode_numpy$lib$histograms, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.index_tricks", modulecode_numpy$lib$index_tricks, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.mixins", modulecode_numpy$lib$mixins, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.nanfunctions", modulecode_numpy$lib$nanfunctions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.npyio", modulecode_numpy$lib$npyio, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.polynomial", modulecode_numpy$lib$polynomial, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.scimath", modulecode_numpy$lib$scimath, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.shape_base", modulecode_numpy$lib$shape_base, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.stride_tricks", modulecode_numpy$lib$stride_tricks, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.twodim_base", modulecode_numpy$lib$twodim_base, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.type_check", modulecode_numpy$lib$type_check, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.ufunclike", modulecode_numpy$lib$ufunclike, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.lib.utils", modulecode_numpy$lib$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.linalg", modulecode_numpy$linalg, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.linalg.linalg", modulecode_numpy$linalg$linalg, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.ma", modulecode_numpy$ma, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.ma.core", modulecode_numpy$ma$core, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.ma.extras", modulecode_numpy$ma$extras, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.ma.mrecords", modulecode_numpy$ma$mrecords, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.matrixlib", modulecode_numpy$matrixlib, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.matrixlib.defmatrix", modulecode_numpy$matrixlib$defmatrix, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.polynomial", modulecode_numpy$polynomial, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.polynomial._polybase", modulecode_numpy$polynomial$_polybase, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.polynomial.chebyshev", modulecode_numpy$polynomial$chebyshev, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.polynomial.hermite", modulecode_numpy$polynomial$hermite, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.polynomial.hermite_e", modulecode_numpy$polynomial$hermite_e, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.polynomial.laguerre", modulecode_numpy$polynomial$laguerre, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.polynomial.legendre", modulecode_numpy$polynomial$legendre, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.polynomial.polynomial", modulecode_numpy$polynomial$polynomial, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.polynomial.polyutils", modulecode_numpy$polynomial$polyutils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.random", modulecode_numpy$random, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.random._pickle", modulecode_numpy$random$_pickle, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"numpy.testing", NULL, 55, 750, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.testing._private", NULL, 56, 186, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"numpy.testing._private.decorators", NULL, 57, 9006, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.testing._private.noseclasses", NULL, 58, 9831, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.testing._private.nosetester", NULL, 59, 14854, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.testing._private.parameterized", NULL, 60, 15576, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.testing._private.utils", NULL, 61, 70141, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"numpy.version", modulecode_numpy$version, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"packaging", modulecode_packaging, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"packaging.__about__", modulecode_packaging$__about__, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip", modulecode_pip, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._internal", modulecode_pip$_internal, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._internal.build_env", modulecode_pip$_internal$build_env, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.cli", modulecode_pip$_internal$cli, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._internal.cli.autocompletion", modulecode_pip$_internal$cli$autocompletion, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.cli.base_command", modulecode_pip$_internal$cli$base_command, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.cli.cmdoptions", modulecode_pip$_internal$cli$cmdoptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.cli.command_context", modulecode_pip$_internal$cli$command_context, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.cli.main", modulecode_pip$_internal$cli$main, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.cli.main_parser", modulecode_pip$_internal$cli$main_parser, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.cli.parser", modulecode_pip$_internal$cli$parser, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.cli.progress_bars", modulecode_pip$_internal$cli$progress_bars, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.cli.spinners", modulecode_pip$_internal$cli$spinners, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.cli.status_codes", modulecode_pip$_internal$cli$status_codes, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.commands", modulecode_pip$_internal$commands, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._internal.configuration", modulecode_pip$_internal$configuration, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.exceptions", modulecode_pip$_internal$exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.index", modulecode_pip$_internal$index, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._internal.index.collector", modulecode_pip$_internal$index$collector, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.index.package_finder", modulecode_pip$_internal$index$package_finder, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.locations", modulecode_pip$_internal$locations, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.models", modulecode_pip$_internal$models, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._internal.models.candidate", modulecode_pip$_internal$models$candidate, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.models.direct_url", modulecode_pip$_internal$models$direct_url, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.models.format_control", modulecode_pip$_internal$models$format_control, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.models.index", modulecode_pip$_internal$models$index, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.models.link", modulecode_pip$_internal$models$link, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.models.scheme", modulecode_pip$_internal$models$scheme, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.models.search_scope", modulecode_pip$_internal$models$search_scope, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.models.selection_prefs", modulecode_pip$_internal$models$selection_prefs, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.models.target_python", modulecode_pip$_internal$models$target_python, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.models.wheel", modulecode_pip$_internal$models$wheel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.network", modulecode_pip$_internal$network, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._internal.network.auth", modulecode_pip$_internal$network$auth, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.network.cache", modulecode_pip$_internal$network$cache, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.network.session", modulecode_pip$_internal$network$session, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.network.utils", modulecode_pip$_internal$network$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.operations", modulecode_pip$_internal$operations, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._internal.operations.build", modulecode_pip$_internal$operations$build, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._internal.operations.build.metadata", modulecode_pip$_internal$operations$build$metadata, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.operations.build.metadata_legacy", modulecode_pip$_internal$operations$build$metadata_legacy, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.operations.freeze", modulecode_pip$_internal$operations$freeze, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.operations.install", modulecode_pip$_internal$operations$install, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._internal.operations.install.editable_legacy", modulecode_pip$_internal$operations$install$editable_legacy, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.operations.install.legacy", modulecode_pip$_internal$operations$install$legacy, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.operations.install.wheel", modulecode_pip$_internal$operations$install$wheel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.pyproject", modulecode_pip$_internal$pyproject, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.req", modulecode_pip$_internal$req, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._internal.req.constructors", modulecode_pip$_internal$req$constructors, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.req.req_file", modulecode_pip$_internal$req$req_file, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.req.req_install", modulecode_pip$_internal$req$req_install, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.req.req_set", modulecode_pip$_internal$req$req_set, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.req.req_uninstall", modulecode_pip$_internal$req$req_uninstall, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils", modulecode_pip$_internal$utils, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._internal.utils.appdirs", modulecode_pip$_internal$utils$appdirs, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.compat", modulecode_pip$_internal$utils$compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.compatibility_tags", modulecode_pip$_internal$utils$compatibility_tags, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.deprecation", modulecode_pip$_internal$utils$deprecation, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.direct_url_helpers", modulecode_pip$_internal$utils$direct_url_helpers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.encoding", modulecode_pip$_internal$utils$encoding, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.entrypoints", modulecode_pip$_internal$utils$entrypoints, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.filesystem", modulecode_pip$_internal$utils$filesystem, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.filetypes", modulecode_pip$_internal$utils$filetypes, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.glibc", modulecode_pip$_internal$utils$glibc, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.hashes", modulecode_pip$_internal$utils$hashes, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.inject_securetransport", modulecode_pip$_internal$utils$inject_securetransport, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.logging", modulecode_pip$_internal$utils$logging, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.misc", modulecode_pip$_internal$utils$misc, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.models", modulecode_pip$_internal$utils$models, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.packaging", modulecode_pip$_internal$utils$packaging, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.pkg_resources", modulecode_pip$_internal$utils$pkg_resources, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.setuptools_build", modulecode_pip$_internal$utils$setuptools_build, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.subprocess", modulecode_pip$_internal$utils$subprocess, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.temp_dir", modulecode_pip$_internal$utils$temp_dir, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.typing", modulecode_pip$_internal$utils$typing, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.unpacking", modulecode_pip$_internal$utils$unpacking, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.urls", modulecode_pip$_internal$utils$urls, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.virtualenv", modulecode_pip$_internal$utils$virtualenv, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.utils.wheel", modulecode_pip$_internal$utils$wheel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.vcs", modulecode_pip$_internal$vcs, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._internal.vcs.bazaar", modulecode_pip$_internal$vcs$bazaar, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.vcs.git", modulecode_pip$_internal$vcs$git, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.vcs.mercurial", modulecode_pip$_internal$vcs$mercurial, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.vcs.subversion", modulecode_pip$_internal$vcs$subversion, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._internal.vcs.versioncontrol", modulecode_pip$_internal$vcs$versioncontrol, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor", modulecode_pip$_vendor, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.appdirs", modulecode_pip$_vendor$appdirs, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.cachecontrol", modulecode_pip$_vendor$cachecontrol, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.cachecontrol.adapter", modulecode_pip$_vendor$cachecontrol$adapter, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.cachecontrol.cache", modulecode_pip$_vendor$cachecontrol$cache, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.cachecontrol.caches", modulecode_pip$_vendor$cachecontrol$caches, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.cachecontrol.caches.file_cache", modulecode_pip$_vendor$cachecontrol$caches$file_cache, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.cachecontrol.caches.redis_cache", modulecode_pip$_vendor$cachecontrol$caches$redis_cache, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.cachecontrol.compat", modulecode_pip$_vendor$cachecontrol$compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.cachecontrol.controller", modulecode_pip$_vendor$cachecontrol$controller, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.cachecontrol.filewrapper", modulecode_pip$_vendor$cachecontrol$filewrapper, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.cachecontrol.serialize", modulecode_pip$_vendor$cachecontrol$serialize, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.cachecontrol.wrapper", modulecode_pip$_vendor$cachecontrol$wrapper, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.certifi", modulecode_pip$_vendor$certifi, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.certifi.core", modulecode_pip$_vendor$certifi$core, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet", modulecode_pip$_vendor$chardet, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.chardet.big5freq", modulecode_pip$_vendor$chardet$big5freq, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.big5prober", modulecode_pip$_vendor$chardet$big5prober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.chardistribution", modulecode_pip$_vendor$chardet$chardistribution, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.charsetgroupprober", modulecode_pip$_vendor$chardet$charsetgroupprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.charsetprober", modulecode_pip$_vendor$chardet$charsetprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.codingstatemachine", modulecode_pip$_vendor$chardet$codingstatemachine, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.cp949prober", modulecode_pip$_vendor$chardet$cp949prober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.enums", modulecode_pip$_vendor$chardet$enums, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.escprober", modulecode_pip$_vendor$chardet$escprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.escsm", modulecode_pip$_vendor$chardet$escsm, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.eucjpprober", modulecode_pip$_vendor$chardet$eucjpprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.euckrfreq", modulecode_pip$_vendor$chardet$euckrfreq, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.euckrprober", modulecode_pip$_vendor$chardet$euckrprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.euctwfreq", modulecode_pip$_vendor$chardet$euctwfreq, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.euctwprober", modulecode_pip$_vendor$chardet$euctwprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.gb2312freq", modulecode_pip$_vendor$chardet$gb2312freq, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.gb2312prober", modulecode_pip$_vendor$chardet$gb2312prober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.hebrewprober", modulecode_pip$_vendor$chardet$hebrewprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.jisfreq", modulecode_pip$_vendor$chardet$jisfreq, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.jpcntx", modulecode_pip$_vendor$chardet$jpcntx, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.langbulgarianmodel", modulecode_pip$_vendor$chardet$langbulgarianmodel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.langgreekmodel", modulecode_pip$_vendor$chardet$langgreekmodel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.langhebrewmodel", modulecode_pip$_vendor$chardet$langhebrewmodel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.langrussianmodel", modulecode_pip$_vendor$chardet$langrussianmodel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.langthaimodel", modulecode_pip$_vendor$chardet$langthaimodel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.langturkishmodel", modulecode_pip$_vendor$chardet$langturkishmodel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.latin1prober", modulecode_pip$_vendor$chardet$latin1prober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.mbcharsetprober", modulecode_pip$_vendor$chardet$mbcharsetprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.mbcsgroupprober", modulecode_pip$_vendor$chardet$mbcsgroupprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.mbcssm", modulecode_pip$_vendor$chardet$mbcssm, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.sbcharsetprober", modulecode_pip$_vendor$chardet$sbcharsetprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.sbcsgroupprober", modulecode_pip$_vendor$chardet$sbcsgroupprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.sjisprober", modulecode_pip$_vendor$chardet$sjisprober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.universaldetector", modulecode_pip$_vendor$chardet$universaldetector, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.utf8prober", modulecode_pip$_vendor$chardet$utf8prober, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.chardet.version", modulecode_pip$_vendor$chardet$version, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.colorama", modulecode_pip$_vendor$colorama, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.colorama.ansi", modulecode_pip$_vendor$colorama$ansi, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.colorama.ansitowin32", modulecode_pip$_vendor$colorama$ansitowin32, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.colorama.initialise", modulecode_pip$_vendor$colorama$initialise, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.colorama.win32", modulecode_pip$_vendor$colorama$win32, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.colorama.winterm", modulecode_pip$_vendor$colorama$winterm, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.contextlib2", modulecode_pip$_vendor$contextlib2, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.distlib", modulecode_pip$_vendor$distlib, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.distlib._backport", modulecode_pip$_vendor$distlib$_backport, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.distlib._backport.sysconfig", modulecode_pip$_vendor$distlib$_backport$sysconfig, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.distlib.compat", modulecode_pip$_vendor$distlib$compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.distlib.resources", modulecode_pip$_vendor$distlib$resources, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.distlib.scripts", modulecode_pip$_vendor$distlib$scripts, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.distlib.util", modulecode_pip$_vendor$distlib$util, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.distro", modulecode_pip$_vendor$distro, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib", modulecode_pip$_vendor$html5lib, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.html5lib._ihatexml", modulecode_pip$_vendor$html5lib$_ihatexml, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib._inputstream", modulecode_pip$_vendor$html5lib$_inputstream, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib._tokenizer", modulecode_pip$_vendor$html5lib$_tokenizer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib._trie", modulecode_pip$_vendor$html5lib$_trie, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.html5lib._trie._base", modulecode_pip$_vendor$html5lib$_trie$_base, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib._trie.py", modulecode_pip$_vendor$html5lib$_trie$py, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib._utils", modulecode_pip$_vendor$html5lib$_utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.constants", modulecode_pip$_vendor$html5lib$constants, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.filters", modulecode_pip$_vendor$html5lib$filters, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.html5lib.filters.alphabeticalattributes", modulecode_pip$_vendor$html5lib$filters$alphabeticalattributes, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.filters.base", modulecode_pip$_vendor$html5lib$filters$base, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.filters.inject_meta_charset", modulecode_pip$_vendor$html5lib$filters$inject_meta_charset, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.filters.optionaltags", modulecode_pip$_vendor$html5lib$filters$optionaltags, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.filters.sanitizer", modulecode_pip$_vendor$html5lib$filters$sanitizer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.filters.whitespace", modulecode_pip$_vendor$html5lib$filters$whitespace, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.html5parser", modulecode_pip$_vendor$html5lib$html5parser, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.serializer", modulecode_pip$_vendor$html5lib$serializer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.treebuilders", modulecode_pip$_vendor$html5lib$treebuilders, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.html5lib.treebuilders.base", modulecode_pip$_vendor$html5lib$treebuilders$base, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.treebuilders.dom", modulecode_pip$_vendor$html5lib$treebuilders$dom, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.treebuilders.etree", modulecode_pip$_vendor$html5lib$treebuilders$etree, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.treebuilders.etree_lxml", modulecode_pip$_vendor$html5lib$treebuilders$etree_lxml, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.treewalkers", modulecode_pip$_vendor$html5lib$treewalkers, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.html5lib.treewalkers.base", modulecode_pip$_vendor$html5lib$treewalkers$base, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.treewalkers.dom", modulecode_pip$_vendor$html5lib$treewalkers$dom, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.treewalkers.etree", modulecode_pip$_vendor$html5lib$treewalkers$etree, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.treewalkers.etree_lxml", modulecode_pip$_vendor$html5lib$treewalkers$etree_lxml, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.html5lib.treewalkers.genshi", modulecode_pip$_vendor$html5lib$treewalkers$genshi, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.idna", modulecode_pip$_vendor$idna, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.idna.core", modulecode_pip$_vendor$idna$core, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.idna.idnadata", modulecode_pip$_vendor$idna$idnadata, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.idna.intranges", modulecode_pip$_vendor$idna$intranges, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.idna.package_data", modulecode_pip$_vendor$idna$package_data, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.idna.uts46data", modulecode_pip$_vendor$idna$uts46data, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.msgpack", modulecode_pip$_vendor$msgpack, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.msgpack._version", modulecode_pip$_vendor$msgpack$_version, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.msgpack.exceptions", modulecode_pip$_vendor$msgpack$exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.msgpack.ext", modulecode_pip$_vendor$msgpack$ext, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.msgpack.fallback", modulecode_pip$_vendor$msgpack$fallback, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.packaging", modulecode_pip$_vendor$packaging, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.packaging.__about__", modulecode_pip$_vendor$packaging$__about__, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.packaging._compat", modulecode_pip$_vendor$packaging$_compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.packaging._structures", modulecode_pip$_vendor$packaging$_structures, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.packaging._typing", modulecode_pip$_vendor$packaging$_typing, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.packaging.markers", modulecode_pip$_vendor$packaging$markers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.packaging.requirements", modulecode_pip$_vendor$packaging$requirements, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.packaging.specifiers", modulecode_pip$_vendor$packaging$specifiers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.packaging.tags", modulecode_pip$_vendor$packaging$tags, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.packaging.utils", modulecode_pip$_vendor$packaging$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.packaging.version", modulecode_pip$_vendor$packaging$version, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.pep517", modulecode_pip$_vendor$pep517, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.pep517.compat", modulecode_pip$_vendor$pep517$compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.pep517.wrappers", modulecode_pip$_vendor$pep517$wrappers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.pkg_resources", modulecode_pip$_vendor$pkg_resources, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.pkg_resources.py31compat", modulecode_pip$_vendor$pkg_resources$py31compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.progress", modulecode_pip$_vendor$progress, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.progress.bar", modulecode_pip$_vendor$progress$bar, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.progress.spinner", modulecode_pip$_vendor$progress$spinner, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.pyparsing", modulecode_pip$_vendor$pyparsing, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests", modulecode_pip$_vendor$requests, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.requests.__version__", modulecode_pip$_vendor$requests$__version__, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests._internal_utils", modulecode_pip$_vendor$requests$_internal_utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests.adapters", modulecode_pip$_vendor$requests$adapters, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests.api", modulecode_pip$_vendor$requests$api, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests.auth", modulecode_pip$_vendor$requests$auth, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests.certs", modulecode_pip$_vendor$requests$certs, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests.compat", modulecode_pip$_vendor$requests$compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests.cookies", modulecode_pip$_vendor$requests$cookies, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests.exceptions", modulecode_pip$_vendor$requests$exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests.hooks", modulecode_pip$_vendor$requests$hooks, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests.models", modulecode_pip$_vendor$requests$models, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests.packages", modulecode_pip$_vendor$requests$packages, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests.sessions", modulecode_pip$_vendor$requests$sessions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests.status_codes", modulecode_pip$_vendor$requests$status_codes, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests.structures", modulecode_pip$_vendor$requests$structures, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.requests.utils", modulecode_pip$_vendor$requests$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.retrying", modulecode_pip$_vendor$retrying, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.six", modulecode_pip$_vendor$six, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.toml", modulecode_pip$_vendor$toml, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.toml.decoder", modulecode_pip$_vendor$toml$decoder, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.toml.encoder", modulecode_pip$_vendor$toml$encoder, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.toml.tz", modulecode_pip$_vendor$toml$tz, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3", modulecode_pip$_vendor$urllib3, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.urllib3._collections", modulecode_pip$_vendor$urllib3$_collections, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3._version", modulecode_pip$_vendor$urllib3$_version, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.connection", modulecode_pip$_vendor$urllib3$connection, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.connectionpool", modulecode_pip$_vendor$urllib3$connectionpool, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.contrib", modulecode_pip$_vendor$urllib3$contrib, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.urllib3.contrib._appengine_environ", modulecode_pip$_vendor$urllib3$contrib$_appengine_environ, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.contrib._securetransport", modulecode_pip$_vendor$urllib3$contrib$_securetransport, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.urllib3.contrib._securetransport.bindings", modulecode_pip$_vendor$urllib3$contrib$_securetransport$bindings, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.contrib._securetransport.low_level", modulecode_pip$_vendor$urllib3$contrib$_securetransport$low_level, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.contrib.pyopenssl", modulecode_pip$_vendor$urllib3$contrib$pyopenssl, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.contrib.securetransport", modulecode_pip$_vendor$urllib3$contrib$securetransport, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.contrib.socks", modulecode_pip$_vendor$urllib3$contrib$socks, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.exceptions", modulecode_pip$_vendor$urllib3$exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.fields", modulecode_pip$_vendor$urllib3$fields, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.filepost", modulecode_pip$_vendor$urllib3$filepost, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.packages", modulecode_pip$_vendor$urllib3$packages, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.urllib3.packages.backports", modulecode_pip$_vendor$urllib3$packages$backports, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.urllib3.packages.backports.makefile", modulecode_pip$_vendor$urllib3$packages$backports$makefile, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.packages.six", modulecode_pip$_vendor$urllib3$packages$six, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.packages.ssl_match_hostname", modulecode_pip$_vendor$urllib3$packages$ssl_match_hostname, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.urllib3.packages.ssl_match_hostname._implementation", modulecode_pip$_vendor$urllib3$packages$ssl_match_hostname$_implementation, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.poolmanager", modulecode_pip$_vendor$urllib3$poolmanager, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.request", modulecode_pip$_vendor$urllib3$request, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.response", modulecode_pip$_vendor$urllib3$response, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.util", modulecode_pip$_vendor$urllib3$util, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.urllib3.util.connection", modulecode_pip$_vendor$urllib3$util$connection, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.util.proxy", modulecode_pip$_vendor$urllib3$util$proxy, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.util.queue", modulecode_pip$_vendor$urllib3$util$queue, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.util.request", modulecode_pip$_vendor$urllib3$util$request, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.util.response", modulecode_pip$_vendor$urllib3$util$response, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.util.retry", modulecode_pip$_vendor$urllib3$util$retry, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.util.ssl_", modulecode_pip$_vendor$urllib3$util$ssl_, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.util.ssltransport", modulecode_pip$_vendor$urllib3$util$ssltransport, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.util.timeout", modulecode_pip$_vendor$urllib3$util$timeout, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.util.url", modulecode_pip$_vendor$urllib3$util$url, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.urllib3.util.wait", modulecode_pip$_vendor$urllib3$util$wait, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.webencodings", modulecode_pip$_vendor$webencodings, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pip._vendor.webencodings.labels", modulecode_pip$_vendor$webencodings$labels, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pip._vendor.webencodings.x_user_defined", modulecode_pip$_vendor$webencodings$x_user_defined, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pkg_resources", NULL, 62, 99833, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"pkg_resources._vendor", NULL, 63, 185, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"pkg_resources._vendor.appdirs", NULL, 64, 20723, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"pkg_resources._vendor.packaging", NULL, 65, 571, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"pkg_resources._vendor.packaging.__about__", NULL, 66, 725, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"pkg_resources._vendor.packaging._compat", NULL, 67, 1142, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"pkg_resources._vendor.packaging._structures", NULL, 68, 2971, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"pkg_resources._vendor.packaging._typing", NULL, 69, 1486, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"pkg_resources._vendor.packaging.markers", NULL, 70, 9295, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"pkg_resources._vendor.packaging.requirements", NULL, 71, 4065, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"pkg_resources._vendor.packaging.specifiers", NULL, 72, 20571, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"pkg_resources._vendor.packaging.utils", NULL, 73, 1645, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"pkg_resources._vendor.packaging.version", NULL, 74, 13194, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"pkg_resources._vendor.pyparsing", NULL, 75, 203194, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"pkg_resources.extern", NULL, 76, 2641, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"prometheus_client", modulecode_prometheus_client, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"prometheus_client.asgi", modulecode_prometheus_client$asgi, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"prometheus_client.context_managers", modulecode_prometheus_client$context_managers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"prometheus_client.decorator", modulecode_prometheus_client$decorator, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"prometheus_client.exposition", modulecode_prometheus_client$exposition, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"prometheus_client.gc_collector", modulecode_prometheus_client$gc_collector, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"prometheus_client.metrics", modulecode_prometheus_client$metrics, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"prometheus_client.metrics_core", modulecode_prometheus_client$metrics_core, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"prometheus_client.mmap_dict", modulecode_prometheus_client$mmap_dict, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"prometheus_client.openmetrics", modulecode_prometheus_client$openmetrics, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"prometheus_client.openmetrics.exposition", modulecode_prometheus_client$openmetrics$exposition, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"prometheus_client.platform_collector", modulecode_prometheus_client$platform_collector, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"prometheus_client.process_collector", modulecode_prometheus_client$process_collector, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"prometheus_client.registry", modulecode_prometheus_client$registry, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"prometheus_client.samples", modulecode_prometheus_client$samples, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"prometheus_client.utils", modulecode_prometheus_client$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"prometheus_client.values", modulecode_prometheus_client$values, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"psutil", modulecode_psutil, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"psutil._common", modulecode_psutil$_common, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"psutil._compat", modulecode_psutil$_compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"psutil._psaix", modulecode_psutil$_psaix, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"psutil._psbsd", modulecode_psutil$_psbsd, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"psutil._pslinux", modulecode_psutil$_pslinux, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"psutil._psosx", modulecode_psutil$_psosx, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"psutil._psposix", modulecode_psutil$_psposix, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"psutil._pssunos", modulecode_psutil$_pssunos, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"psutil._pswindows", modulecode_psutil$_pswindows, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1", modulecode_pyasn1, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pyasn1.codec", modulecode_pyasn1$codec, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pyasn1.codec.ber", modulecode_pyasn1$codec$ber, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pyasn1.codec.ber.decoder", modulecode_pyasn1$codec$ber$decoder, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.codec.ber.encoder", modulecode_pyasn1$codec$ber$encoder, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.codec.ber.eoo", modulecode_pyasn1$codec$ber$eoo, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.codec.cer", modulecode_pyasn1$codec$cer, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pyasn1.codec.cer.decoder", modulecode_pyasn1$codec$cer$decoder, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.codec.cer.encoder", modulecode_pyasn1$codec$cer$encoder, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.codec.der", modulecode_pyasn1$codec$der, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pyasn1.codec.der.decoder", modulecode_pyasn1$codec$der$decoder, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.codec.der.encoder", modulecode_pyasn1$codec$der$encoder, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.compat", modulecode_pyasn1$compat, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pyasn1.compat.binary", modulecode_pyasn1$compat$binary, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.compat.calling", modulecode_pyasn1$compat$calling, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.compat.dateandtime", modulecode_pyasn1$compat$dateandtime, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.compat.integer", modulecode_pyasn1$compat$integer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.compat.octets", modulecode_pyasn1$compat$octets, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.compat.string", modulecode_pyasn1$compat$string, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.debug", modulecode_pyasn1$debug, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.error", modulecode_pyasn1$error, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.type", modulecode_pyasn1$type, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pyasn1.type.base", modulecode_pyasn1$type$base, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.type.char", modulecode_pyasn1$type$char, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.type.constraint", modulecode_pyasn1$type$constraint, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.type.error", modulecode_pyasn1$type$error, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.type.namedtype", modulecode_pyasn1$type$namedtype, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.type.namedval", modulecode_pyasn1$type$namedval, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.type.opentype", modulecode_pyasn1$type$opentype, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.type.tag", modulecode_pyasn1$type$tag, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.type.tagmap", modulecode_pyasn1$type$tagmap, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.type.univ", modulecode_pyasn1$type$univ, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1.type.useful", modulecode_pyasn1$type$useful, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1_modules", modulecode_pyasn1_modules, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pyasn1_modules.pem", modulecode_pyasn1_modules$pem, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1_modules.rfc2251", modulecode_pyasn1_modules$rfc2251, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1_modules.rfc2459", modulecode_pyasn1_modules$rfc2459, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyasn1_modules.rfc5208", modulecode_pyasn1_modules$rfc5208, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyrsistent", modulecode_pyrsistent, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"pyrsistent._checked_types", modulecode_pyrsistent$_checked_types, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyrsistent._field_common", modulecode_pyrsistent$_field_common, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyrsistent._helpers", modulecode_pyrsistent$_helpers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyrsistent._immutable", modulecode_pyrsistent$_immutable, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyrsistent._pbag", modulecode_pyrsistent$_pbag, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyrsistent._pclass", modulecode_pyrsistent$_pclass, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyrsistent._pdeque", modulecode_pyrsistent$_pdeque, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyrsistent._plist", modulecode_pyrsistent$_plist, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyrsistent._pmap", modulecode_pyrsistent$_pmap, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyrsistent._precord", modulecode_pyrsistent$_precord, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyrsistent._pset", modulecode_pyrsistent$_pset, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyrsistent._pvector", modulecode_pyrsistent$_pvector, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyrsistent._toolz", modulecode_pyrsistent$_toolz, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"pyrsistent._transformations", modulecode_pyrsistent$_transformations, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray", modulecode_ray, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray._private", modulecode_ray$_private, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray._private.client_mode_hook", modulecode_ray$_private$client_mode_hook, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.compat", modulecode_ray$_private$compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.function_manager", modulecode_ray$_private$function_manager, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.import_thread", modulecode_ray$_private$import_thread, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.memory_monitor", modulecode_ray$_private$memory_monitor, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.parameter", modulecode_ray$_private$parameter, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.ray_logging", modulecode_ray$_private$ray_logging, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.runtime_env", modulecode_ray$_private$runtime_env, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.services", modulecode_ray$_private$services, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.signature", modulecode_ray$_private$signature, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.thirdparty", modulecode_ray$_private$thirdparty, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray._private.thirdparty.pathspec", modulecode_ray$_private$thirdparty$pathspec, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray._private.thirdparty.pathspec.compat", modulecode_ray$_private$thirdparty$pathspec$compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.thirdparty.pathspec.pathspec", modulecode_ray$_private$thirdparty$pathspec$pathspec, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.thirdparty.pathspec.pattern", modulecode_ray$_private$thirdparty$pathspec$pattern, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.thirdparty.pathspec.patterns", modulecode_ray$_private$thirdparty$pathspec$patterns, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray._private.thirdparty.pathspec.patterns.gitwildmatch", modulecode_ray$_private$thirdparty$pathspec$patterns$gitwildmatch, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.thirdparty.pathspec.util", modulecode_ray$_private$thirdparty$pathspec$util, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray._private.utils", modulecode_ray$_private$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.actor", modulecode_ray$actor, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler", modulecode_ray$autoscaler, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.autoscaler._private", modulecode_ray$autoscaler$_private, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.autoscaler._private._kubernetes", modulecode_ray$autoscaler$_private$_kubernetes, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.autoscaler._private._kubernetes.config", modulecode_ray$autoscaler$_private$_kubernetes$config, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private._kubernetes.node_provider", modulecode_ray$autoscaler$_private$_kubernetes$node_provider, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.autoscaler", modulecode_ray$autoscaler$_private$autoscaler, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.aws", modulecode_ray$autoscaler$_private$aws, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.autoscaler._private.aws.config", modulecode_ray$autoscaler$_private$aws$config, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.aws.node_provider", modulecode_ray$autoscaler$_private$aws$node_provider, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.aws.utils", modulecode_ray$autoscaler$_private$aws$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.azure", modulecode_ray$autoscaler$_private$azure, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.autoscaler._private.azure.config", modulecode_ray$autoscaler$_private$azure$config, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.azure.node_provider", modulecode_ray$autoscaler$_private$azure$node_provider, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.cli_logger", modulecode_ray$autoscaler$_private$cli_logger, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.cluster_dump", modulecode_ray$autoscaler$_private$cluster_dump, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.command_runner", modulecode_ray$autoscaler$_private$command_runner, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.commands", modulecode_ray$autoscaler$_private$commands, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.constants", modulecode_ray$autoscaler$_private$constants, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.docker", modulecode_ray$autoscaler$_private$docker, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.event_summarizer", modulecode_ray$autoscaler$_private$event_summarizer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.event_system", modulecode_ray$autoscaler$_private$event_system, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.gcp", modulecode_ray$autoscaler$_private$gcp, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.autoscaler._private.gcp.config", modulecode_ray$autoscaler$_private$gcp$config, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.gcp.node_provider", modulecode_ray$autoscaler$_private$gcp$node_provider, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.legacy_info_string", modulecode_ray$autoscaler$_private$legacy_info_string, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.load_metrics", modulecode_ray$autoscaler$_private$load_metrics, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.local", modulecode_ray$autoscaler$_private$local, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.autoscaler._private.local.config", modulecode_ray$autoscaler$_private$local$config, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.local.coordinator_node_provider", modulecode_ray$autoscaler$_private$local$coordinator_node_provider, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.local.node_provider", modulecode_ray$autoscaler$_private$local$node_provider, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.log_timer", modulecode_ray$autoscaler$_private$log_timer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.node_launcher", modulecode_ray$autoscaler$_private$node_launcher, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.node_tracker", modulecode_ray$autoscaler$_private$node_tracker, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.prom_metrics", modulecode_ray$autoscaler$_private$prom_metrics, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.providers", modulecode_ray$autoscaler$_private$providers, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.resource_demand_scheduler", modulecode_ray$autoscaler$_private$resource_demand_scheduler, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.staroid", modulecode_ray$autoscaler$_private$staroid, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.autoscaler._private.staroid.command_runner", modulecode_ray$autoscaler$_private$staroid$command_runner, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.staroid.node_provider", modulecode_ray$autoscaler$_private$staroid$node_provider, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.subprocess_output_util", modulecode_ray$autoscaler$_private$subprocess_output_util, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.updater", modulecode_ray$autoscaler$_private$updater, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler._private.util", modulecode_ray$autoscaler$_private$util, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler.aws", modulecode_ray$autoscaler$aws, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.autoscaler.azure", modulecode_ray$autoscaler$azure, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.autoscaler.command_runner", modulecode_ray$autoscaler$command_runner, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler.gcp", modulecode_ray$autoscaler$gcp, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.autoscaler.kubernetes", modulecode_ray$autoscaler$kubernetes, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.autoscaler.local", modulecode_ray$autoscaler$local, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.autoscaler.node_provider", modulecode_ray$autoscaler$node_provider, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler.sdk", modulecode_ray$autoscaler$sdk, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.autoscaler.staroid", modulecode_ray$autoscaler$staroid, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.autoscaler.tags", modulecode_ray$autoscaler$tags, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.client_builder", modulecode_ray$client_builder, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.cloudpickle", modulecode_ray$cloudpickle, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.cloudpickle.cloudpickle", modulecode_ray$cloudpickle$cloudpickle, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.cloudpickle.cloudpickle_fast", modulecode_ray$cloudpickle$cloudpickle_fast, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.cloudpickle.compat", modulecode_ray$cloudpickle$compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.core", modulecode_ray$core, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.core.generated", modulecode_ray$core$generated, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.core.generated.common_pb2", modulecode_ray$core$generated$common_pb2, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.core.generated.gcs_pb2", modulecode_ray$core$generated$gcs_pb2, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.core.generated.gcs_service_pb2", modulecode_ray$core$generated$gcs_service_pb2, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.core.generated.gcs_service_pb2_grpc", modulecode_ray$core$generated$gcs_service_pb2_grpc, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.core.generated.node_manager_pb2", modulecode_ray$core$generated$node_manager_pb2, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.core.generated.node_manager_pb2_grpc", modulecode_ray$core$generated$node_manager_pb2_grpc, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.core.generated.ray_client_pb2", modulecode_ray$core$generated$ray_client_pb2, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.core.generated.ray_client_pb2_grpc", modulecode_ray$core$generated$ray_client_pb2_grpc, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.cross_language", modulecode_ray$cross_language, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.exceptions", modulecode_ray$exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.experimental", modulecode_ray$experimental, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.experimental.dynamic_resources", modulecode_ray$experimental$dynamic_resources, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.experimental.internal_kv", modulecode_ray$experimental$internal_kv, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.experimental.packaging", modulecode_ray$experimental$packaging, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.experimental.packaging.load_package", modulecode_ray$experimental$packaging$load_package, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.external_storage", modulecode_ray$external_storage, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.gcs_utils", modulecode_ray$gcs_utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.internal", modulecode_ray$internal, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.internal.internal_api", modulecode_ray$internal$internal_api, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.job_config", modulecode_ray$job_config, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.new_dashboard", modulecode_ray$new_dashboard, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.new_dashboard.actor_utils", modulecode_ray$new_dashboard$actor_utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.new_dashboard.consts", modulecode_ray$new_dashboard$consts, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.new_dashboard.datacenter", modulecode_ray$new_dashboard$datacenter, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.new_dashboard.memory_utils", modulecode_ray$new_dashboard$memory_utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.new_dashboard.modules", modulecode_ray$new_dashboard$modules, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.new_dashboard.modules.stats_collector", modulecode_ray$new_dashboard$modules$stats_collector, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.new_dashboard.modules.stats_collector.stats_collector_consts", modulecode_ray$new_dashboard$modules$stats_collector$stats_collector_consts, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.new_dashboard.modules.stats_collector.stats_collector_head", modulecode_ray$new_dashboard$modules$stats_collector$stats_collector_head, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.new_dashboard.utils", modulecode_ray$new_dashboard$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.node", modulecode_ray$node, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.profiling", modulecode_ray$profiling, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.ray_constants", modulecode_ray$ray_constants, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.remote_function", modulecode_ray$remote_function, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.resource_spec", modulecode_ray$resource_spec, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.runtime_context", modulecode_ray$runtime_context, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.serialization", modulecode_ray$serialization, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.serialization_addons", modulecode_ray$serialization_addons, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.state", modulecode_ray$state, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util", modulecode_ray$util, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.util.actor_pool", modulecode_ray$util$actor_pool, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.check_serialize", modulecode_ray$util$check_serialize, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client", modulecode_ray$util$client, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.util.client.api", modulecode_ray$util$client$api, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client.client_pickler", modulecode_ray$util$client$client_pickler, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client.common", modulecode_ray$util$client$common, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client.dataclient", modulecode_ray$util$client$dataclient, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client.logsclient", modulecode_ray$util$client$logsclient, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client.options", modulecode_ray$util$client$options, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client.runtime_context", modulecode_ray$util$client$runtime_context, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client.server", modulecode_ray$util$client$server, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.util.client.server.dataservicer", modulecode_ray$util$client$server$dataservicer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client.server.logservicer", modulecode_ray$util$client$server$logservicer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client.server.proxier", modulecode_ray$util$client$server$proxier, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client.server.server", modulecode_ray$util$client$server$server, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client.server.server_pickler", modulecode_ray$util$client$server$server_pickler, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client.server.server_stubs", modulecode_ray$util$client$server$server_stubs, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client.worker", modulecode_ray$util$client$worker, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.client_connect", modulecode_ray$util$client_connect, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.debug", modulecode_ray$util$debug, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.inspect", modulecode_ray$util$inspect, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.iter", modulecode_ray$util$iter, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.iter_metrics", modulecode_ray$util$iter_metrics, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.placement_group", modulecode_ray$util$placement_group, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.rpdb", modulecode_ray$util$rpdb, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.serialization", modulecode_ray$util$serialization, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.timer", modulecode_ray$util$timer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.util.tracing", modulecode_ray$util$tracing, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"ray.util.tracing.tracing_helper", modulecode_ray$util$tracing$tracing_helper, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"ray.worker", modulecode_ray$worker, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"redis", modulecode_redis, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"redis._compat", modulecode_redis$_compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"redis.client", modulecode_redis$client, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"redis.connection", modulecode_redis$connection, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"redis.exceptions", modulecode_redis$exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"redis.lock", modulecode_redis$lock, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"redis.utils", modulecode_redis$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests", modulecode_requests, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"requests.__version__", modulecode_requests$__version__, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests._internal_utils", modulecode_requests$_internal_utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests.adapters", modulecode_requests$adapters, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests.api", modulecode_requests$api, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests.auth", modulecode_requests$auth, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests.certs", modulecode_requests$certs, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests.compat", modulecode_requests$compat, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests.cookies", modulecode_requests$cookies, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests.exceptions", modulecode_requests$exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests.hooks", modulecode_requests$hooks, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests.models", modulecode_requests$models, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests.packages", modulecode_requests$packages, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests.sessions", modulecode_requests$sessions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests.status_codes", modulecode_requests$status_codes, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests.structures", modulecode_requests$structures, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"requests.utils", modulecode_requests$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"rsa", modulecode_rsa, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"rsa.asn1", modulecode_rsa$asn1, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"rsa.common", modulecode_rsa$common, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"rsa.core", modulecode_rsa$core, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"rsa.key", modulecode_rsa$key, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"rsa.parallel", modulecode_rsa$parallel, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"rsa.pem", modulecode_rsa$pem, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"rsa.pkcs1", modulecode_rsa$pkcs1, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"rsa.prime", modulecode_rsa$prime, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"rsa.randnum", modulecode_rsa$randnum, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"rsa.transform", modulecode_rsa$transform, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"setuptools", NULL, 77, 8525, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"setuptools._deprecation_warning", NULL, 78, 540, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools._imp", NULL, 79, 2080, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.archive_util", NULL, 80, 5700, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.command", NULL, 81, 709, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"setuptools.command.bdist_egg", NULL, 82, 13236, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.command.bdist_rpm", NULL, 83, 1313, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.command.develop", NULL, 84, 6361, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.command.easy_install", NULL, 85, 63208, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.command.egg_info", NULL, 86, 21452, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.command.install", NULL, 87, 3991, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.command.install_scripts", NULL, 88, 2320, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.command.py36compat", NULL, 89, 4561, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.command.sdist", NULL, 90, 7325, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.command.setopt", NULL, 91, 4555, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.config", NULL, 92, 18937, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.depends", NULL, 93, 5161, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.dist", NULL, 94, 32393, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.extension", NULL, 95, 1912, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.extern", NULL, 96, 2664, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"setuptools.glob", NULL, 97, 3673, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.installer", NULL, 98, 2739, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.monkey", NULL, 99, 4634, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.namespaces", NULL, 100, 3600, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.package_index", NULL, 101, 32798, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.py34compat", NULL, 102, 474, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.sandbox", NULL, 103, 15594, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.ssl_support", NULL, 104, 6754, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.unicode_utils", NULL, 105, 1096, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.version", NULL, 106, 281, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.wheel", NULL, 107, 7134, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"setuptools.windows_support", NULL, 108, 1003, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG},
    {"six", modulecode_six, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"starlette", modulecode_starlette, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"starlette.concurrency", modulecode_starlette$concurrency, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"starlette.datastructures", modulecode_starlette$datastructures, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"starlette.types", modulecode_starlette$types, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"sumo_integration", modulecode_sumo_integration, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"sumo_integration.bridge_helper", modulecode_sumo_integration$bridge_helper, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"sumo_integration.carla_simulation", modulecode_sumo_integration$carla_simulation, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"sumo_integration.constants", modulecode_sumo_integration$constants, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"sumo_integration.sumo_simulation", modulecode_sumo_integration$sumo_simulation, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"typing_extensions", modulecode_typing_extensions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3", modulecode_urllib3, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"urllib3._collections", modulecode_urllib3$_collections, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3._version", modulecode_urllib3$_version, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.connection", modulecode_urllib3$connection, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.connectionpool", modulecode_urllib3$connectionpool, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.contrib", modulecode_urllib3$contrib, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"urllib3.contrib._appengine_environ", modulecode_urllib3$contrib$_appengine_environ, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.contrib.appengine", modulecode_urllib3$contrib$appengine, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.contrib.pyopenssl", modulecode_urllib3$contrib$pyopenssl, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.contrib.socks", modulecode_urllib3$contrib$socks, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.exceptions", modulecode_urllib3$exceptions, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.fields", modulecode_urllib3$fields, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.filepost", modulecode_urllib3$filepost, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.packages", modulecode_urllib3$packages, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"urllib3.packages.backports", modulecode_urllib3$packages$backports, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"urllib3.packages.backports.makefile", modulecode_urllib3$packages$backports$makefile, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.packages.six", modulecode_urllib3$packages$six, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.packages.ssl_match_hostname", modulecode_urllib3$packages$ssl_match_hostname, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"urllib3.packages.ssl_match_hostname._implementation", modulecode_urllib3$packages$ssl_match_hostname$_implementation, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.poolmanager", modulecode_urllib3$poolmanager, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.request", modulecode_urllib3$request, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.response", modulecode_urllib3$response, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.util", modulecode_urllib3$util, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"urllib3.util.connection", modulecode_urllib3$util$connection, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.util.proxy", modulecode_urllib3$util$proxy, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.util.queue", modulecode_urllib3$util$queue, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.util.request", modulecode_urllib3$util$request, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.util.response", modulecode_urllib3$util$response, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.util.retry", modulecode_urllib3$util$retry, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.util.ssl_", modulecode_urllib3$util$ssl_, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.util.ssltransport", modulecode_urllib3$util$ssltransport, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.util.timeout", modulecode_urllib3$util$timeout, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.util.url", modulecode_urllib3$util$url, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"urllib3.util.wait", modulecode_urllib3$util$wait, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util", modulecode_util, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"util.classes", modulecode_util$classes, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"util.classes.constants", modulecode_util$classes$constants, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util.classes.errors", modulecode_util$classes$errors, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util.classes.messages", modulecode_util$classes$messages, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util.classes.perceived_objects", modulecode_util$classes$perceived_objects, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util.classes.sensor_data", modulecode_util$classes$sensor_data, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util.classes.sensor_loaded_vehicle", modulecode_util$classes$sensor_loaded_vehicle, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util.classes.utils", modulecode_util$classes$utils, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util.create_sumo_vtypes", modulecode_util$create_sumo_vtypes, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util.func", modulecode_util$func, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util.my_netconvert_carla", modulecode_util$my_netconvert_carla, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util.netconvert_carla", modulecode_util$netconvert_carla, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util.ray", modulecode_util$ray, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"util.ray.messages", modulecode_util$ray$messages, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util.ray.perceived_objects", modulecode_util$ray$perceived_objects, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util.ray.sensor_data", modulecode_util$ray$sensor_data, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"util.sequential_types", modulecode_util$sequential_types, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"wheel", NULL, 109, 191, NUITKA_TRANSLATED_FLAG | NUITKA_BYTECODE_FLAG | NUITKA_PACKAGE_FLAG},
    {"yaml", modulecode_yaml, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"yaml.composer", modulecode_yaml$composer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.constructor", modulecode_yaml$constructor, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.cyaml", modulecode_yaml$cyaml, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.dumper", modulecode_yaml$dumper, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.emitter", modulecode_yaml$emitter, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.error", modulecode_yaml$error, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.events", modulecode_yaml$events, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.loader", modulecode_yaml$loader, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.nodes", modulecode_yaml$nodes, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.parser", modulecode_yaml$parser, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.reader", modulecode_yaml$reader, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.representer", modulecode_yaml$representer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.resolver", modulecode_yaml$resolver, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.scanner", modulecode_yaml$scanner, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.serializer", modulecode_yaml$serializer, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yaml.tokens", modulecode_yaml$tokens, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yarl", modulecode_yarl, 0, 0, NUITKA_TRANSLATED_FLAG | NUITKA_PACKAGE_FLAG},
    {"yarl._quoting", modulecode_yarl$_quoting, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yarl._quoting_py", modulecode_yarl$_quoting_py, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"yarl._url", modulecode_yarl$_url, 0, 0, NUITKA_TRANSLATED_FLAG},
    {"zipp", modulecode_zipp, 0, 0, NUITKA_TRANSLATED_FLAG},
    {NULL, NULL, 0, 0, 0}
};

static void _loadBytesCodesBlob()
{
    static bool init_done = false;

    if (init_done == false) {
        loadConstantsBlob((PyObject **)bytecode_data, ".bytecode");

        init_done = true;
    }
}


void setupMetaPathBasedLoader(void) {
    static bool init_done = false;
    if (init_done == false) {
        _loadBytesCodesBlob();
        registerMetaPathBasedUnfreezer(meta_path_loader_entries, bytecode_data);

        init_done = true;
    }


}

// This provides the frozen (compiled bytecode) files that are included if
// any.

// These modules should be loaded as bytecode. They may e.g. have to be loadable
// during "Py_Initialize" already, or for irrelevance, they are only included
// in this un-optimized form. These are not compiled by Nuitka, and therefore
// are not accelerated at all, merely bundled with the binary or module, so
// that CPython library can start out finding them.

struct frozen_desc {
    char const *name;
    int index;
    int size;
};

static struct frozen_desc _frozen_modules[] = {

    {NULL, 0, 0}
};


void copyFrozenModulesTo(struct _frozen *destination) {
    _loadBytesCodesBlob();

    struct frozen_desc *current = _frozen_modules;

    for (;;) {
        destination->name = (char *)current->name;
        destination->code = bytecode_data[current->index];
        destination->size = current->size;

        if (destination->name == NULL) break;

        current += 1;
        destination += 1;
    };
}


