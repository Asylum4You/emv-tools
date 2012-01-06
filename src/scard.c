#include <stdlib.h>
#include <winscard.h>
//#undef SCARD_AUTOALLOCATE

#include "scard.h"

struct sc {
	SCARDCONTEXT hContext;
	SCARDHANDLE hCard;
	LONG rv;
	LPSTR rfunc;
	LPSTR mszReaders;
	LPCSCARD_IO_REQUEST pioSendPci;
};

#define CHECK(sc, func, ret) \
	if ((sc)->rv != SCARD_S_SUCCESS) { \
		(sc)->rfunc = (func); \
		return ret; \
	}

struct sc *scard_init(void)
{
	struct sc *sc = calloc(1, sizeof(*sc));
	DWORD dwReaders;

	sc->rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &sc->hContext);
	CHECK(sc, "SCardEstablishContext", sc);

#ifdef SCARD_AUTOALLOCATE
	dwReaders = SCARD_AUTOALLOCATE;

	sc->rv = SCardListReaders(sc->hContext, NULL, (LPSTR)&sc->mszReaders, &dwReaders);
	CHECK(sc, "SCardListReaders", sc);
#else
	sc->rv = SCardListReaders(sc->hContext, NULL, NULL, &dwReaders);
	CHECK(sc, "SCardListReaders", sc);

	sc->mszReaders = calloc(dwReaders, sizeof(char));
	sc->rv = SCardListReaders(sc->hContext, NULL, sc->mszReaders, &dwReaders);
	CHECK(sc, "SCardListReaders", sc);
#endif

	return sc;
}

void scard_shutdown(struct sc **psc)
{
	struct sc *sc = *psc;
	// FIXME: disconect if necessary
#ifdef SCARD_AUTOALLOCATE
	sc->rv = SCardFreeMemory(sc->hContext, sc->mszReaders);
	CHECK(sc, "SCardFreeMemory", );
#else
	free(sc->mszReaders);
#endif

	sc->rv = SCardReleaseContext(sc->hContext);
	CHECK(sc, "SCardReleaseContext", );

	free(sc);
	*psc = NULL;
}

bool scard_is_error(struct sc *sc)
{
	return sc && (sc->rv != SCARD_S_SUCCESS);
}

#include "stdio.h"
const char *scard_error(struct sc *sc)
{
	printf("%s: %s\n", sc->rfunc, pcsc_stringify_error(sc->rv));
	return "\n"; // FIXME
}

void scard_connect(struct sc *sc)
{
	DWORD dwActiveProtocol;
	sc->rv = SCardConnect(sc->hContext, sc->mszReaders, SCARD_SHARE_EXCLUSIVE,
		SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &sc->hCard, &dwActiveProtocol);
	CHECK(sc, "SCardConnect", );

	switch(dwActiveProtocol)
	{
	case SCARD_PROTOCOL_T0:
		sc->pioSendPci = SCARD_PCI_T0;
		break;

	case SCARD_PROTOCOL_T1:
		sc->pioSendPci = SCARD_PCI_T1;
		break;
	default:
		sc->rv = SCARD_E_INVALID_VALUE;
		CHECK(sc, "SCardSelectProtocol", );
	}
}

void scard_disconnect(struct sc *sc)
{
	sc->rv = SCardDisconnect(sc->hCard, SCARD_RESET_CARD);
	CHECK(sc, "SCardDisconnect", );
}

int scard_transmit(struct sc *sc,
		const unsigned char *inbuf, size_t inlen,
		unsigned char *outbuf, size_t outlen)
{
	DWORD dwRecvLength = outlen;
	sc->rv = SCardTransmit(sc->hCard, sc->pioSendPci,
			inbuf, inlen, NULL, outbuf, &dwRecvLength);
	CHECK(sc, "SCardTransmit", 0);

	return dwRecvLength;
}
