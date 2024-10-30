#include <iostream>
#include <cctype>
#include <string>
#include <cassert>

using namespace std;

bool isValidUppercaseStateCode(string stateCode)
{
    const string codes =
        "AL.AK.AZ.AR.CA.CO.CT.DE.DC.FL.GA.HI.ID.IL.IN.IA.KS."
        "KY.LA.ME.MD.MA.MI.MN.MS.MO.MT.NE.NV.NJ.NH.NM.NY.NC."
        "ND.OH.OK.OR.PA.RI.SC.SD.TN.TX.UT.VT.VA.WA.WV.WI.WY";
    return (stateCode.size() == 2 &&
        stateCode.find('.') == string::npos &&  // no '.' in stateCode
        codes.find(stateCode) != string::npos);  // match found
}

bool hasRightSyntax(string pollData)
{
    if (pollData.empty())
        return true;

    size_t pos = 0;
    while (pos < pollData.length())
    {
        // Check party code (must be a letter)
        if (!isalpha(pollData.at(pos)))
            return false;
        pos++;

        // Must have at least one more character for digit
        if (pos >= pollData.length())
            return false;

        // Must have at least one digit
        if (!isdigit(pollData.at(pos)))
            return false;

        // Handle optional second digit
        if (pos + 1 < pollData.length() && isdigit(pollData.at(pos + 1)))
            pos += 2;
        else
            pos++;

        // Must have two more characters for state code
        if (pos + 1 >= pollData.length())
            return false;

        // Check state code
        string stateCode = "";
        stateCode += toupper(pollData.at(pos));
        stateCode += toupper(pollData.at(pos + 1));

        if (!isValidUppercaseStateCode(stateCode))
            return false;

        pos += 2;
    }

    return true;
}

int computeVotes(string pollData, char party, int& voteCount)
{
    // Save original voteCount
    int originalVoteCount = voteCount;

    // First check if party is a letter
    if (!isalpha(party))
        return 3;

    // Check syntax
    if (!hasRightSyntax(pollData))
    {
        voteCount = originalVoteCount;
        return 1;
    }

    // Handle empty string case
    if (pollData.empty())
    {
        voteCount = 0;
        return 0;
    }

    // Initialize vote count
    voteCount = 0;

    size_t pos = 0;
    while (pos < pollData.length())
    {
        // Get party for this prediction
        char currentParty = pollData.at(pos);
        pos++;

        // Convert digits to number
        int stateVotes = pollData.at(pos) - '0';  // First digit
        pos++;

        // Check for second digit
        if (pos < pollData.length() && isdigit(pollData.at(pos)))
        {
            stateVotes = stateVotes * 10 + (pollData.at(pos) - '0');
            pos++;
        }

        // Check for zero votes
        if (stateVotes == 0)
        {
            voteCount = originalVoteCount;
            return 2;
        }

        // Add votes if party matches (case insensitive)
        if (toupper(currentParty) == toupper(party))
            voteCount += stateVotes;

        // Skip state code
        pos += 2;
    }

    return 0;
}

int main()
{
    // Empty string cases
    assert(hasRightSyntax(""));  // Empty string must be valid

    int votes;
    votes = 666;
    assert(computeVotes("", 'D', votes) == 0 && votes == 0);  // Empty string with valid party
    votes = 666;
    assert(computeVotes("", '@', votes) == 3 && votes == 666);  // Empty string with invalid party
    votes = 666;
    assert(computeVotes("", '0', votes) == 3 && votes == 666);  // Empty string with digit party
    votes = 666;
    assert(computeVotes("", ' ', votes) == 3 && votes == 666);  // Empty string with space party

    // Basic single prediction format
    assert(hasRightSyntax("D5CA"));    // Basic valid: letter-digit-state
    assert(hasRightSyntax("D15CA"));   // Two digits valid
    assert(hasRightSyntax("D05CA"));   // Leading zero valid
    assert(hasRightSyntax("D99CA"));   // Maximum two digits
    assert(!hasRightSyntax("DCA"));    // Missing vote count
    assert(!hasRightSyntax("5CA"));    // Missing party code
    assert(!hasRightSyntax("D5C"));    // Incomplete state code
    assert(!hasRightSyntax("DD5CA"));  // Two letters in a row
    assert(!hasRightSyntax("D5CAD"));  // Incomplete after valid prediction

    // Letter case handling
    assert(hasRightSyntax("d5CA"));    // Lowercase party valid
    assert(hasRightSyntax("D5ca"));    // Lowercase state valid
    votes = -999;
    assert(computeVotes("d5CA", 'D', votes) == 0 && votes == 5);  // Party case insensitive
    votes = -999;
    assert(computeVotes("D5CA", 'd', votes) == 0 && votes == 5);  // Parameter case insensitive

    // Multiple predictions
    assert(hasRightSyntax("D5CAR4NY"));       // Two predictions
    assert(hasRightSyntax("D5CAR4NYL3CT"));   // Three predictions
    assert(!hasRightSyntax("D5CAD"));         // Incomplete after valid
    assert(!hasRightSyntax("D5CARD4TX"));     // Missing separation

    // Error conditions
    votes = -999;
    assert(computeVotes("D5CA", '@', votes) == 3 && votes == -999);  // Must be 3 for invalid party
    votes = -999;
    assert(computeVotes("@5CA", 'D', votes) == 1 && votes == -999);  // Must be 1 for invalid syntax
    votes = -999;
    assert(computeVotes("D0CA", 'D', votes) == 2 && votes == -999);  // Zero votes

    // Test various invalid party characters
    votes = -999;
    assert(computeVotes("D5CA", '0', votes) == 3 && votes == -999);  // Digit party
    votes = -999;
    assert(computeVotes("D5CA", '!', votes) == 3 && votes == -999);  // Symbol party
    votes = -999;
    assert(computeVotes("D5CA", ' ', votes) == 3 && votes == -999);  // Space party

    // Vote counting including edge cases
    votes = -999;
    assert(computeVotes("R99TXD99CA", 'D', votes) == 0 && votes == 99);  // Max two digits
    votes = -999;
    assert(computeVotes("R40TXD54CAr6MS", 'D', votes) == 0 && votes == 54);  // Only count D
    votes = -999;
    assert(computeVotes("R40TXD54CAr6MS", 'R', votes) == 0 && votes == 46);  // Count both R and r
    votes = -999;
    assert(computeVotes("D5CAD4NY", 'D', votes) == 0 && votes == 9);  // Sum multiple predictions
    votes = -999;
    assert(computeVotes("R40TXD54CAr6MS", 'L', votes) == 0 && votes == 0);  // No matching predictions

    // Multiple errors
    votes = -999;
    int result = computeVotes("@5CA", '@', votes);
    assert((result == 1 || result == 3) && votes == -999);  // Invalid party and syntax

    votes = -999;
    result = computeVotes("D0CX", 'D', votes);
    assert((result == 1 || result == 2) && votes == -999);  // Invalid syntax and zero votes

    // State codes
    assert(hasRightSyntax("D5DC"));    // DC is valid
    assert(hasRightSyntax("D5NY"));    // NY is valid
    assert(hasRightSyntax("D5Ca"));    // Mixed case state
    assert(!hasRightSyntax("D5XX"));   // Invalid state
    assert(!hasRightSyntax("D5C."));   // Punctuation in state
    assert(!hasRightSyntax("D5C9"));   // Number in state

    cout << "All tests succeeded" << endl;
    return 0;
}