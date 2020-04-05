#include <crypto/CfbCrypto.h>
#include <cstring>
#include <gtest/gtest.h>
#include <string>

TEST(CfbCrypto, ECMA376Standard_deriveKey) {
  odr::crypto::CfbCrypto::EncryptionHeader encryptionHeader{};
  encryptionHeader.algId = 0x660e;
  encryptionHeader.algIdHash = 0x8004;
  encryptionHeader.keySize = 128;
  encryptionHeader.providerType = 0x18;
  odr::crypto::CfbCrypto::EncryptionVerifier encryptionVerifier{};
  encryptionVerifier.saltSize = 16;
  std::memcpy(encryptionVerifier.salt,
              "\xe8\x82"
              "fI\x0c[\xd1\xee\xbd+C\x94\xe3\xf8"
              "0\xef",
              sizeof(encryptionVerifier.salt));
  const std::string password = "Password1234_";
  const std::string expectedKey =
      "@\xb1:q\xf9\x0b\x96n7T\x08\xf2\xd1\x81\xa1\xaa";

  odr::crypto::CfbCrypto::ECMA376Standard crypto(encryptionHeader,
                                                 encryptionVerifier, "");
  const std::string key = crypto.deriveKey(password);

  EXPECT_TRUE(key == expectedKey);
}

TEST(CfbCrypto, ECMA376Standard_verify) {
  const odr::crypto::CfbCrypto::EncryptionHeader encryptionHeader{};
  odr::crypto::CfbCrypto::EncryptionVerifier encryptionVerifier{};
  encryptionVerifier.verifierHashSize = 16;
  std::memcpy(encryptionVerifier.encryptedVerifier,
              "Qos.\x96o\xac\x17\xb1\xc5\xd7\xd8\xcc"
              "6\xc9(",
              sizeof(encryptionVerifier.encryptedVerifier));
  const std::string encryptedVerifierHash =
      "+ah\xda\xbe)\x11\xad+\xd3|\x17"
      "Ft\\\x14\xd3\xcf\x1b\xb1@\xa4\x8fNo=#\x88\x08r\xb1j";
  const std::string key = "@\xb1:q\xf9\x0b\x96n7T\x08\xf2\xd1\x81\xa1\xaa";

  odr::crypto::CfbCrypto::ECMA376Standard crypto(
      encryptionHeader, encryptionVerifier, encryptedVerifierHash);
  EXPECT_TRUE(crypto.verify(key));
}
