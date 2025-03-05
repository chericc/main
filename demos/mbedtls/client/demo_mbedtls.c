#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/platform.h"

#define SERVER_NAME "10.0.0.3"
#define SERVER_PORT "8883"
#define DEBUG_LEVEL 4

const char *pers = "ssl_client1";



const char s_my_aws_key[] = 
"-----BEGIN PRIVATE KEY-----\n"
"MIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCuIIFzI3Y0oKeb\n"
"X67u+K8IJiwhrKxTj8F53FYMWqGNkRw4IoOXWAVTIkuZMXq/+EoFS5THnMGV9PUn\n"
"mGZ9mybj5Sm7ccixF0PJw04srwXqYvzgEAjZnV2mKDaHRbRJ+pgOYqMs8VANFd5x\n"
"E3WjTPJU4g5dN7nzxyCxyFTBtgNuV7/s4v7od6jHZRVGCFoI+gmcS5R6HJATxBBG\n"
"yJs4vB8QHP6CI0Acu7K3dTK+aQvcbTNXGplA3bjG1u1/pa3HvGb/0XYq9Ro1O9H7\n"
"qPb1F3TYLYUPH6Hjbl6xC2u32BL4+Nj1sgFKbg/bR6VtfuCBhqueSW3Q+fuWrq1b\n"
"XH1H54sNAgMBAAECggEAAokOIXlNdhA1yI0g3LFpyQZXQ6lCPBg+grH63cSvEALi\n"
"JI4aGOjK8Q4xiBpAlY19tGyIjsyBRBf0Pt6Y9XC+JPjX/fb2618xmT07BOsKfMJy\n"
"uKd7HYINVEdHHfqdmKspriHxCs07s2kTW1Wjc5yB9crx1xkh38MRPo1ycHygOanO\n"
"xZnkZq07MuQe5mY4scP0vuZeaMx5wSv+WZF+ZGMYAzyq2yac61FK0DUHs1rk5uVo\n"
"GEyyj0FCoe9JMldD73mGwpalOTe+o/iDgqHg009/jCNSP4Fp3CQ0wfEAjr/TBrfx\n"
"RZ4gaOlZiwP4M22KBfYgXjkLGH4ZVcIy9RRHzjDuOQKBgQDdUiIcdUz3P/uYeJcp\n"
"omgU0NPnoY3DFZ9Kq6mhCrItfkTKHZl9ZsdyhAfrRh27iyZwkcHbYxu4Ds/w8pNn\n"
"XmCrGYScDLRe/WOZfy4GFb/I9cZHn9pcAn1GSFg4VzJL8YK96KbAkbROMcwQNFYE\n"
"H37f90wk+XC4yBLBGNiNF5zQFQKBgQDJaUexAILr+tFutnjgdWtBOW3M3BODyBZH\n"
"gDOT7rrlnB/xX4Qs1OIKRy/eiehi0UtS6Z6H0meOyND2FBcYzsalIfMH5UDWHzBr\n"
"r/ObtkbgGBfckTsdWsmNVi+/NvZ0MC4k06kf2QzMoV01vtReKPtvARYDr09cMnb6\n"
"M9SnBk+VGQKBgFpNTMW3xrOLU6iea279aOI1YoC8Fg9nHxtkd03SA2k0f/THpvzX\n"
"lsFOxbxlW3RODl0X27Ub9w9eW9G/6AzhISguvUC+nkmzsZPLpIevEBV60nAe9QVd\n"
"rqeilPxH96ms3N+HsaIY0SymfNCPyhjckPPo/xSHAksityQ6cwD+hWkRAoGAW6sz\n"
"Gfhlngm2MHGUanSGsFT3CHpR13TFzx0mXHoXTlHUqAMVIWJyzsK/osWZowAnDEGQ\n"
"CblVsMLteFKEgKH7/0SIXFuXOt1PKHQYqBI6B9xvX9Ap255qhY1ohdr6KL+VAOXk\n"
"373l+WyoA8Iq3gBBD7Cq2Fp+ADRnulDExJA2wGECgYAIQz03DlTNIcgr3yYQ1NdJ\n"
"5BE3vO/c4iL87iq9fO6KlIlcrSUDAYv02hy2qPDYt1Uz69/RJzf2BYPeEYDxACkE\n"
"bA9kn6xJZXAoXTZpBVRR+t0HhojVyw9bu+CqHdcizhLCOhYyPtyrTnwlKr+ufOBL\n"
"MeOpU6DKFWmMMSo4juP5NA==\n"
"-----END PRIVATE KEY-----\n"
;

const char s_my_aws_cert[] = 
"-----BEGIN CERTIFICATE-----\n"
"MIIDETCCAfkCFCG/XKNFIgMCLgO8BftGTenzB/N3MA0GCSqGSIb3DQEBCwUAMEUx\n"
"CzAJBgNVBAYTAkNOMRMwEQYDVQQIDApTb21lLVN0YXRlMSEwHwYDVQQKDBhJbnRl\n"
"cm5ldCBXaWRnaXRzIFB0eSBMdGQwHhcNMjUwMzA1MDIzOTE2WhcNMjYwMzA1MDIz\n"
"OTE2WjBFMQswCQYDVQQGEwJBVTETMBEGA1UECAwKU29tZS1TdGF0ZTEhMB8GA1UE\n"
"CgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMIIBIjANBgkqhkiG9w0BAQEFAAOC\n"
"AQ8AMIIBCgKCAQEAriCBcyN2NKCnm1+u7vivCCYsIaysU4/BedxWDFqhjZEcOCKD\n"
"l1gFUyJLmTF6v/hKBUuUx5zBlfT1J5hmfZsm4+Upu3HIsRdDycNOLK8F6mL84BAI\n"
"2Z1dpig2h0W0SfqYDmKjLPFQDRXecRN1o0zyVOIOXTe588cgschUwbYDble/7OL+\n"
"6Heox2UVRghaCPoJnEuUehyQE8QQRsibOLwfEBz+giNAHLuyt3UyvmkL3G0zVxqZ\n"
"QN24xtbtf6Wtx7xm/9F2KvUaNTvR+6j29Rd02C2FDx+h425esQtrt9gS+PjY9bIB\n"
"Sm4P20elbX7ggYarnklt0Pn7lq6tW1x9R+eLDQIDAQABMA0GCSqGSIb3DQEBCwUA\n"
"A4IBAQAjPuPcHiO1yJaPtocs+nDRnSURRKyDC6dVB8AVP3g+Y2oMSlCM52bTf8uo\n"
"SaW0DVzu2OnmnIC9j0tlMAUi41yd7ZDuY0Rxhwa3Bp4qLXl4k5qbKn0cNjhBSy2T\n"
"2LwVzeROts7fEPzVT09VY0nYm3zRXtH/j3PoGxDrkBelY02/cJaDIHBHGdrEa/Ow\n"
"pqer9enHQJw+Q8tXsLMLNM9ywfqlpLtiJysmUkuBvlYePHHwEKv8xLZpsPtDpR4z\n"
"r+KRSHIxYEeedSpDd3qHpCOjhGVpoD/pLgbRPvTRRJtww1M2WmAXXdYO1VUjRYmm\n"
"p+pgIlnTIoeck3wx9DtPlj6fYrqF\n"
"-----END CERTIFICATE-----\n"
;

const char s_my_aws_ca[] = 
"-----BEGIN CERTIFICATE-----\n"
"MIIDazCCAlOgAwIBAgIUAX5mKbyHlH+ovYnN1FF1bG9Yc3kwDQYJKoZIhvcNAQEL\n"
"BQAwRTELMAkGA1UEBhMCQ04xEzARBgNVBAgMClNvbWUtU3RhdGUxITAfBgNVBAoM\n"
"GEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDAeFw0yNTAzMDUwMjM4MzFaFw0yNjAz\n"
"MDUwMjM4MzFaMEUxCzAJBgNVBAYTAkNOMRMwEQYDVQQIDApTb21lLVN0YXRlMSEw\n"
"HwYDVQQKDBhJbnRlcm5ldCBXaWRnaXRzIFB0eSBMdGQwggEiMA0GCSqGSIb3DQEB\n"
"AQUAA4IBDwAwggEKAoIBAQCsmJtnWyQkLRscSS06DgCN2VUc1vsbWO3ZFs0qLOiq\n"
"bFSMwrevg6KH5R5K396fpDDG/V+lG4bp24A3WHVdaNm+8HDjWzMWw7eXFfTt92Gw\n"
"panfEWdwiAzbPWMuPCdxCJsW7RZe86w8dK2qN6dWnXwRP+y6yr31k6TkH5NSpRc9\n"
"HqJwpbKbB0JlCciIYW8I4cYVYwFlKqBPsow7+6TONq6XpHFaa5bkxI674XhTRi/o\n"
"FeU8JvL3L5IeIt/LkosdJMGTUQQOH/QIfMIjdc3oBR1AyXJ71XV1h4iQocZNvXdw\n"
"ji9Em5qUCSAJQc0wz18DWa8KBWvTtN8imnKpJIKBZ1cPAgMBAAGjUzBRMB0GA1Ud\n"
"DgQWBBQ2rPnOHmVa45aDQS+lqhggbCqLADAfBgNVHSMEGDAWgBQ2rPnOHmVa45aD\n"
"QS+lqhggbCqLADAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQCj\n"
"7LDKhxGEKt08ScnEr7m8e9uNbP3/a52reQWF2Guvql05Yj74e/CiR2iJ8QoPfLC3\n"
"l3uDi5oHkT9pGyJa/OvsXatbUBhXtvB+wXmoke52kBD8QZHzR6X+U8/KvG4lj1Ru\n"
"TtdmAED4oMgIFGxetML4zWwWwSKf/nWJHavYXY8Nr5ibSP5uU7d25RxllYMeDNeg\n"
"CbJcXNJdRLug0MqbP3IrUrmiAM/hO21PXu4eScXbH3OKFkcZN2fSXaDahKmXhQoF\n"
"+cePzWYz7KtzZNqtCB2oyKKqp2IQbLEdCOIlKuZKuuUqZTgpsA+G8ogJgyKql21g\n"
"F+oqOFWW5O6IXJQ0NEx0\n"
"-----END CERTIFICATE-----\n"
;

const char *aws_iot_cert_get(unsigned int *strlength)
{
	*strlength = sizeof(s_my_aws_cert);
	return s_my_aws_cert;
}

const char *aws_iot_root_ca_get(unsigned int *strlength)
{
	*strlength = sizeof(s_my_aws_ca);
	return s_my_aws_ca;
}

const char *aws_iot_private_key_get(unsigned int *strlength)
{
	*strlength = sizeof(s_my_aws_key);
	return s_my_aws_key;
}

int main(void)
{
    int ret;
    int exit_code = MBEDTLS_EXIT_FAILURE;
    mbedtls_net_context server_fd;
    uint32_t flags;
    unsigned char buf[1024];
    const char *pers = "ssl_client1";
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
    mbedtls_x509_crt clicert;
    mbedtls_pk_context pkey;

    mbedtls_net_init(&server_fd);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_x509_crt_init(&clicert);
    mbedtls_pk_init(&pkey);

    /*
     * Load the trusted CA
     */
    printf("  . Loading the CA root certificate ...");
    fflush(stdout);

    unsigned int buflen = 0;
    const unsigned char *buf_cc = NULL;

    buf_cc = aws_iot_root_ca_get(&buflen);
    printf("calen=%d\n", buflen);
    ret = mbedtls_x509_crt_parse(&cacert, buf_cc, buflen);
    // ret = mbedtls_x509_crt_parse_file(&cacert, "/home/test/opensrc/mosquito/mosquitto-2.0.20/build-host/output/sbin/ca.crt");
    if (ret != 0)
    {
        printf(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
        goto exit;
    }

    printf(" ok (%d skipped)\n", ret);

    /*
     * Load the client certificate
     */
    printf("  . Loading the client cert. and key...");
    fflush(stdout);

    buf_cc = aws_iot_cert_get(&buflen);
    printf("calen=%d\n", buflen);
    ret = mbedtls_x509_crt_parse(&clicert, buf_cc, buflen);
    // ret = mbedtls_x509_crt_parse_file(&clicert, "/home/test/opensrc/mosquito/mosquitto-2.0.20/build-host/output/sbin/client.crt");
    if (ret != 0)
    {
        printf(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
        goto exit;
    }

    buf_cc = aws_iot_private_key_get(&buflen);
    printf("calen=%d\n", buflen);
    ret = mbedtls_pk_parse_key(&pkey, buf_cc, buflen, NULL, 0);
    // ret = mbedtls_pk_parse_keyfile(&pkey, "/home/test/opensrc/mosquito/mosquitto-2.0.20/build-host/output/sbin/client.key", NULL);
    if (ret != 0)
    {
        printf(" failed\n  !  mbedtls_pk_parse_key returned -0x%x\n\n", -ret);
        goto exit;
    }

    printf(" ok\n");

    /*
     * Set up the SSL/TLS structure
     */
    printf("  . Setting up the SSL/TLS structure...");
    fflush(stdout);

    if ((ret = mbedtls_ssl_config_defaults(&conf,
                                            MBEDTLS_SSL_IS_CLIENT,
                                            MBEDTLS_SSL_TRANSPORT_STREAM,
                                            MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        printf(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
        goto exit;
    }

    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_own_cert(&conf, &clicert, &pkey);

    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char *) pers,
                                     strlen(pers))) != 0)
    {
        printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
        goto exit;
    }

    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        printf(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
        goto exit;
    }

    if ((ret = mbedtls_ssl_set_hostname(&ssl, SERVER_NAME)) != 0)
    {
        printf(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
        goto exit;
    }

    /*
     * Connect to the TCP socket
     */
    printf("  . Connecting to tcp/%s/%s...", SERVER_NAME, SERVER_PORT);
    fflush(stdout);

    if ((ret = mbedtls_net_connect(&server_fd, SERVER_NAME,
                                   SERVER_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
    {
        printf(" failed\n  ! mbedtls_net_connect returned %d\n\n", ret);
        goto exit;
    }

    printf(" ok\n");

    /*
     * Perform the SSL/TLS handshake
     */
    printf("  . Performing the SSL/TLS handshake...");
    fflush(stdout);

    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
    {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
            goto exit;
        }
    }

    printf(" ok\n");

    /*
     * Verify the server certificate
     */
    printf("  > Verifying peer X.509 certificate...");

    /* In real life, we probably want to bail out when ret != 0 */
    if ((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
    {
        char vrfy_buf[512];

        printf(" failed\n");

        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);

        printf("%s\n", vrfy_buf);
    }
    else
        printf(" ok\n");

    /*
     * Write the GET request
     */
    printf("  > Write to server:");
    fflush(stdout);

    const char *request = "GET / HTTP/1.1\r\nHost: " SERVER_NAME "\r\n\r\n";

    ret = mbedtls_ssl_write(&ssl, (unsigned char *) request, strlen(request));
    if (ret <= 0)
    {
        printf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
        goto exit;
    }

    int len = ret;
    printf(" %d bytes written\n\n%s", len, (char *) buf);

    /*
     * Read the HTTP response
     */
    printf("  < Read from server:");
    fflush(stdout);

    do
    {
        len = sizeof(buf) - 1;
        memset(buf, 0, sizeof(buf));
        ret = mbedtls_ssl_read(&ssl, buf, len);

        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
            continue;

        if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
            break;

        if (ret < 0)
        {
            printf("failed\n  ! mbedtls_ssl_read returned %d\n\n", ret);
            break;
        }

        if (ret == 0)
        {
            printf("\n\nEOF\n\n");
            break;
        }

        len = ret;
        printf(" %d bytes read\n\n%s", len, (char *) buf);
    }
    while (1);

    exit_code = MBEDTLS_EXIT_SUCCESS;

exit:
#ifdef MBEDTLS_ERROR_C
    if (exit_code != MBEDTLS_EXIT_SUCCESS)
    {
        char error_buf[100];
        mbedtls_strerror(ret, error_buf, sizeof(error_buf));
        printf("Last error was: %s\n", error_buf);
    }
#endif

    mbedtls_net_free(&server_fd);
    mbedtls_x509_crt_free(&cacert);
    mbedtls_x509_crt_free(&clicert);
    mbedtls_pk_free(&pkey);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    return exit_code;
}
