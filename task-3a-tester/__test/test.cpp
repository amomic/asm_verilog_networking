#include "framework/userspace.h"

#include <cstdio>
#include <fstream>
#include <functional>
#include <iomanip>
#include <optional>
#include <string_view>
#include <sstream>
#include <vector>

#include <unistd.h>
#include <sys/mman.h>

namespace _CONTest {

    struct CaptureStdOut {
        CaptureStdOut() { capture(); }
        ~CaptureStdOut() { decapture(); }
        void capture()
        {
            if (_capturing) return;

            fflush(stdout);
            _orig = dup(STDOUT_FILENO);
            if (_orig <= 0)
                throw std::string("failed to clone stdout");

            int buf = memfd_create("stdout_buf", 0);
            if (buf <= 0)
                throw std::string("failed to create memory-backed buffer");
            
            dup2(buf, STDOUT_FILENO);
            close(buf);

            _capturing = true;
        }
        std::string decapture()
        {
            if (!_capturing) return "";

            fflush(stdout);
            int buf = dup(STDOUT_FILENO);
            dup2(_orig, STDOUT_FILENO);
            close(_orig);
            _capturing = false;

            if (buf <= 0)
                return "<failed to retrieve buffer>";

            off_t size = lseek(buf, 0, SEEK_END);
            lseek(buf, 0, SEEK_SET);
            
            std::string result;
            result.resize(size);

            char* readTo = result.data();
            while (size > 0)
            {
                ssize_t nRead = read(buf, readTo, size);
                if (nRead < 0)
                    return "<failed to retrieve buffer>";
                readTo += nRead;
                size -= nRead;
            }

            return result;
        }
        bool _capturing = false;
        int _orig;
    };
    
    template <typename... T>
    std::string concatenate(T ...args)
    {
        std::stringstream ss;
        ((ss << args), ...);
        return ss.str();
    }

    std::vector<uint8_t> GetFileContents(char const* path)
    {
        std::vector<uint8_t> data;
        std::ifstream input(concatenate("__test/input/", path), std::ios::in | std::ios::binary);
        if (!input)
            throw concatenate("Failed to open input file ", path, ".");
        input.seekg(0, std::ios::end);
        data.resize(input.tellg());
        input.seekg(0, std::ios::beg);
        input.read(reinterpret_cast<char*>(data.data()), data.size());
        return data;
    }

    FilterResponse GetResponse(char const* inputFile, Direction direction)
    {
        auto input = GetFileContents(inputFile);
        return processPacket(input.data(), input.size(), direction);
    }

    std::function<void()> ExpectBooleanResultTest(char const* inputFile, Direction direction, FilterResponse expectedResponse)
    {
        return [=](){
            FilterResponse response = GetResponse(inputFile, direction);
            if (expectedResponse.accept != response.accept)
            {
                if (!response.accept)
                    throw concatenate("Expected ACCEPT for ", inputFile, ", got DROP.");
                else
                    throw concatenate("Expected DROP for ", inputFile, ", got ACCEPT.");
            }
            if (response.overridePacketData)
            {
                if (!response.accept)
                    throw std::string("Got DROP with override packet data?");
                else if (GetFileContents(inputFile) != *response.overridePacketData)
                    throw concatenate("Expected plain ACCEPT for ", inputFile, ", got content-modified ACCEPT.");
            }
        };
    }
    
    std::function<void()> ExpectPacketLossRatioTest(int expectedN, int numTests, std::vector<std::pair<char const*, Direction>> const& inputs)
    {
        return [=](){
            int n = 0;
            for (int i=0; i<numTests; ++i)
            {
                auto const& input = inputs[i%inputs.size()];
                n += !GetResponse(input.first, input.second).accept;
            }
            if (n != expectedN)
                throw concatenate("Expected to lose ", expectedN, " of 12 packets, but lost ", n, " of ", numTests, " instead.");
        };
    }

    std::function<void()> ExpectModificationResultTest(char const* inputFile, Direction direction, char const* expectedOutputFile, std::optional<std::function<char const*(size_t)>> hintFn = std::nullopt)
    {
        return [=](){
            FilterResponse response = GetResponse(inputFile, direction);
            if (!response.accept)
                throw std::string("Expected content-modified ACCEPT, got DROP.");
            if (!response.overridePacketData)
                throw std::string("Expected content-modified ACCEPT, got plain ACCEPT.");
            
            auto expectedData = GetFileContents(expectedOutputFile);
            auto const& providedData = *response.overridePacketData;
            size_t const n = expectedData.size();
            if (n != providedData.size())
                throw concatenate("Length mismatch, expected ", n, " bytes, got ", providedData.size(), " bytes.");
            
            std::optional<size_t> firstMismatch = std::nullopt;

            size_t const cutoff = ((40 < n) && (expectedData[40] == providedData[40])) ? 40 : 0;
            size_t i=cutoff; /* skip ip/tcp header and inspect payload first... */
            for (; i<n; ++i)
            {
                if (expectedData[i] == providedData[i])
                {
                    if (firstMismatch)
                        break;
                }
                else if (!firstMismatch)
                    firstMismatch = i;
            }
            
            if (cutoff && !firstMismatch)
            { /* now check the ip/tcp header too... */
                i=0;
                for (; i<cutoff; ++i)
                {
                    if (expectedData[i] == providedData[i])
                    {
                        if (firstMismatch)
                            break;
                    }
                    else if (!firstMismatch)
                        firstMismatch = i;
                }
            }

            if (!firstMismatch) /* entire packet checked, all good */
                return;
            
            size_t mismatchLength = i - (*firstMismatch);
            std::stringstream errMsg;
            errMsg << "Mismatch, starting at index " << *firstMismatch << ", length " << mismatchLength << "." << std::endl << "Expected:";
            size_t printLength = std::min(mismatchLength, size_t(6));
            for (size_t j=0; j<printLength; ++j)
                errMsg << " " << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << uint32_t(expectedData[(*firstMismatch)+j]);
            if (mismatchLength > 6)
                errMsg << " ..";
            errMsg << std::endl << "Got:     ";
            for (size_t j=0; j<printLength; ++j)
                errMsg << " " << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << uint32_t(providedData[(*firstMismatch)+j]);
            if (mismatchLength > 6)
                errMsg << " ..";
            
            if (hintFn)
            {
                char const* hint = (*hintFn)(*firstMismatch);
                if (hint)
                    errMsg << std::endl << "Hint:     " << hint;
            }
            throw errMsg.str();
        };
    }

}

int main(int,char const**)
{
    using namespace _CONTest;

    size_t nPass = 0, nFail = 0;
    auto RunTest = [&nPass, &nFail](char const* name, std::function<void()> fn)
    {
        printf("\nRunning test \"%s\"...\n", name);
        CaptureStdOut cap;
        std::optional<std::string> failure = std::nullopt;
        try
        {
            fn();
        }
        catch (std::string const& problem)
        {
            failure.emplace(problem);
        }


        std::string output = cap.decapture();

        if (!failure)
        {
            ++nPass;
            printf("Test \"%s\" \033[32mpassed\033[0m - OK!\n", name);
            std::ofstream(concatenate("test_output/PASS -- ", name, ".txt"), std::ios::out | std::ios::binary) << output;
        }
        else
        {
            ++nFail;
            printf("(!!) Test \"%s\" \033[31mfailed\033[0m, error below:\n%s\n", name, failure->c_str());
            std::ofstream(concatenate("test_output/FAIL -- ", name, ".txt"), std::ios::out | std::ios::binary) << output;
            printf("(!!) %zu bytes of stdout have been flushed to \"test_output/FAIL -- %s.txt\" for your convenience.\n", output.size(), name);
        }
    };
    
    RunTest("ICMP ping to 8.8.4.4", ExpectBooleanResultTest("8844.bin", OUTGOING, DROP));
    RunTest("ICMP pings to 172.19.0.5", ExpectPacketLossRatioTest(4, 12, { { "1721905_1.bin", OUTGOING }, { "1721905_2.bin", OUTGOING }, { "1721905_3.bin", OUTGOING } }));
    RunTest("ICMP pings to 172.10.11.12", ExpectPacketLossRatioTest(0, 12, { { "172101112_1.bin", OUTGOING }, { "172101112_2.bin", OUTGOING }, { "172101112_3.bin", OUTGOING } }));
    RunTest("ICMP pings from 172.26.156.199", ExpectPacketLossRatioTest(3, 12, {{ "17226156199_1.bin", INCOMING }, { "17226156199_2.bin", INCOMING }, { "17226156199_3.bin", INCOMING } }));
    
    RunTest("HTTP connection to port 80", []()
    {
        try {
            ExpectPacketLossRatioTest(2, 4, {{ "HTTP_GET.bin", OUTGOING }})();
            fprintf(stderr, "HTTP connection has 50%% packet loss - this is the \033[36mbase task\033[0m.\n");
        } catch (std::string const& e1) {
            /* base task test failed, maybe this is bonus task? */
            try {
                ExpectBooleanResultTest("HTTP_REPLY.bin", INCOMING, ACCEPT)();
                ExpectModificationResultTest("HTTP_REPLY_2.bin", INCOMING, "HTTP_REPLY_2.mod.bin", [](size_t idx) -> char const* {
                    switch (idx) {
                        case 36: case 37: return "That's an incorrect TCP checksum!";
                        case 75: case 76: case 77: case 78: case 79: case 80: return "That doesn't look like the right kind of fruit...";
                        default: return nullptr;
                    }
                })();
                fprintf(stderr, "HTTP response correctly modified - this is the \033[36mbonus task\033[0m.\n");
            } catch (std::string const& e2) {
                throw concatenate("Base task failed:  ", e1, "\nBonus task failed: ", e2);
            }
        }
    });

    RunTest("HTTPS connection to en.wikipedia.org", []()
    {
        try {
            ExpectBooleanResultTest("HTTPS_HELLO_en.wikipedia.org.bin", OUTGOING, DROP)();
            fprintf(stderr, "HTTPS connection blocked - this is the \033[36mbase task\033[0m.\n");
        } catch (std::string const& e1) {
            /* is this the bonus task? */
            try {
                /* handshake with disallowed en.wikipedia.org - this should be interrupted */
                ExpectModificationResultTest("HTTPS_HELLO_en.wikipedia.org.bin", OUTGOING, "HTTPS_HELLO_en.wikipedia.org.mod.bin", [](size_t idx) -> char const* {
                    if ((36 <= idx) && (idx <= 37)) return "That's an incorrect TCP checksum!";
                    if ((130 <= idx) && (idx <= 131)) return "You've got to overwrite the cipher suites with 0xEAEA!";
                    if ((132 <= idx) && (idx <= 191)) return "Make sure you are overwriting ALL the cipher suites!";
                    if ((192 <= idx) && (idx <= 254)) return "You're overwriting too much. The cipher suites length field is in bytes!";
                    return nullptr;
                })();
                fprintf(stderr, "HTTPS ClientHello correctly modified - this is the \033[36mbonus task\033[0m.\n");
            } catch (std::string const& e2) {
                throw concatenate("Base task failed:  ", e1, "\nBonus task failed: ", e2);
            }
        }
    });

    RunTest("HTTPS connection to de.wikipedia.org", []()
    {
        try {
            ExpectBooleanResultTest("Full HTTPS Request - de.wikipedia.org/pkt_o001.bin", OUTGOING, DROP)();
            fprintf(stderr, "HTTPS connection blocked - this is the \033[36mbase task\033[0m.\n");
        } catch (std::string const& e1) {
            try {
                /* full request cycle from allowed de.wikipedia.org - this should succeed */
                for (size_t i=1; i<=125; ++i)
                {
                    std::string budgetToStringOfI = concatenate(char('0'+(i/100%10)), char('0'+(i/10%10)), char('0'+(i%10)));
                    try {
                        ExpectBooleanResultTest(concatenate("Full HTTPS Request - de.wikipedia.org/pkt_i", budgetToStringOfI, ".bin").c_str(), INCOMING, ACCEPT)();
                    } catch (std::string const& e) {
                        if (e.rfind("Failed to open input file", 0) == std::string::npos)
                            throw;
                        
                        ExpectBooleanResultTest(concatenate("Full HTTPS Request - de.wikipedia.org/pkt_o", budgetToStringOfI, ".bin").c_str(), OUTGOING, ACCEPT)();
                    }
                }
            } catch (std::string const& e2) {
                throw concatenate("Base task failed:  ", e1, "\nBonus task failed: ", e2);
            }
        }
    });
    
    printf("\n\n== TEST SUMMARY: ==\n%2zu passed, %2zu failed\n\n", nPass, nFail);
    
    return 0;
}
