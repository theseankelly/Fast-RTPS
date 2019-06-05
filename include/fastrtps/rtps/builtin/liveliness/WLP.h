// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file WLP.h
 *
 */

#ifndef WLP_H_
#define WLP_H_
#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC
#include <vector>

#include "../../common/Time_t.h"
#include "../../common/Locator.h"
#include "../../common/Guid.h"
#include "../../../qos/QosPolicies.h"

namespace eprosima {
namespace fastrtps{

class ReaderQos;
class WriterQos;

namespace rtps {

class BuiltinProtocols;
class LivelinessManager;
class ReaderHistory;
class ReaderProxyData;
class RTPSParticipantImpl;
class RTPSReader;
class RTPSWriter;
class StatefulReader;
class StatefulWriter;
class ParticipantProxyData;
class WLivelinessPeriodicAssertion;
class WLPListener;
class WriterHistory;
class WriterProxyData;

/**
 * Class WLP that implements the Writer Liveliness Protocol described in the RTPS specification.
 * @ingroup LIVELINESS_MODULE
 */
class WLP
{
	friend class WLPListener;
	friend class WLivelinessPeriodicAssertion;
    friend class StatefulReader;
    friend class StatelessReader;

public:
	/**
	* Constructor
	* @param prot Pointer to the BuiltinProtocols object.
	*/
	WLP(BuiltinProtocols* prot);
	virtual ~WLP();
	/**
	 * Initialize the WLP protocol.
	 * @param p Pointer to the RTPS participant implementation.
	 * @return true if the initialziacion was succesful.
	 */
	bool initWL(RTPSParticipantImpl* p);
	/**
	 * Assign the remote endpoints for a newly discovered RTPSParticipant.
	 * @param pdata Pointer to the RTPSParticipantProxyData object.
	 * @return True if correct.
	 */
	bool assignRemoteEndpoints(const ParticipantProxyData& pdata);
	/**
	 * Remove remote endpoints from the liveliness protocol.
	 * @param pdata Pointer to the ParticipantProxyData to remove
	 */
	void removeRemoteEndpoints(ParticipantProxyData* pdata);
	/**
	 * Add a local writer to the liveliness protocol.
	 * @param W Pointer to the RTPSWriter.
	 * @param wqos Quality of service policies for the writer.
    * @return True if correct.
	 */
    bool add_local_writer(RTPSWriter* W, const WriterQos& wqos);
	/**
	 * Remove a local writer from the liveliness protocol.
	 * @param W Pointer to the RTPSWriter.
	 * @return True if removed.
	 */
    bool remove_local_writer(RTPSWriter* W);

    /**
     * @brief Adds a local reader to the liveliness protocol
     * @param reader Pointer to the RTPS reader
     * @param rqos Quality of service policies for the reader
     * @return True if added successfully
     */
    bool add_local_reader(RTPSReader* reader, const ReaderQos& rqos);

    /**
     * @brief Removes a local reader from the livliness protocol
     * @param reader Pointer to the reader to remove
     * @return True if removed successfully
     */
    bool remove_local_reader(RTPSReader* reader);

    /**
     * @brief Asserts liveliness of writers with given kind
     * @param kind The liveliness kind
     * @return True if liveliness was asserted
     */
    bool assert_liveliness(LivelinessQosPolicyKind kind);

    /**
     * @brief A method to assert liveliness of a given writer
     * @param writer The writer, specified via its id
     * @param kind The writer liveliness kind
     * @param lease_duration The writer lease duration
     * @return True if liveliness was asserted
     */
    bool assert_liveliness(
            GUID_t writer,
            LivelinessQosPolicyKind kind,
            Duration_t lease_duration);

	/**
	 * Get the builtin protocols
	 * @return Builtin protocols
	 */
	BuiltinProtocols* getBuiltinProtocols(){return mp_builtinProtocols;};

    /**
     * Get the livelines builtin writer
     * @return stateful writer
     */
    StatefulWriter* getBuiltinWriter();

    /**
    * Get the livelines builtin writer's history
    * @return writer history
    */
    WriterHistory* getBuiltinWriterHistory();
	
#if HAVE_SECURITY
    bool pairing_remote_reader_with_local_writer_after_security(const GUID_t& local_writer,
        const ReaderProxyData& remote_reader_data);

    bool pairing_remote_writer_with_local_reader_after_security(const GUID_t& local_reader,
        const WriterProxyData& remote_writer_data);
#endif

private:
    /**
     * Create the endpoitns used in the WLP.
     * @return true if correct.
     */
    bool createEndpoints();

    /**
     * Get the RTPS participant
     * @return RTPS participant
     */
    inline RTPSParticipantImpl* getRTPSParticipant(){return mp_participant;}

    //! Minimum time among liveliness periods of automatic writers, in milliseconds
    double min_automatic_ms_;
    //! Minimum time among liveliness periods of manual by participant writers, in milliseconds
    double min_manual_by_participant_ms_;
    //!Pointer to the local RTPSParticipant.
	RTPSParticipantImpl* mp_participant;
	//!Pointer to the builtinprotocol class.
	BuiltinProtocols* mp_builtinProtocols;
	//!Pointer to the builtinRTPSParticipantMEssageWriter.
	StatefulWriter* mp_builtinWriter;
	//!Pointer to the builtinRTPSParticipantMEssageReader.
	StatefulReader* mp_builtinReader;
	//!Writer History
	WriterHistory* mp_builtinWriterHistory;
	//!Reader History
	ReaderHistory* mp_builtinReaderHistory;
    //!Listener object.
    WLPListener* mp_listener;
    //!Pointer to the periodic assertion timer object for automatic liveliness writers
    WLivelinessPeriodicAssertion* automatic_liveliness_assertion_;
    //!Pointer to the periodic assertion timer object for manual by participant liveliness writers
    WLivelinessPeriodicAssertion* manual_liveliness_assertion_;
    //! List of the writers using automatic liveliness.
    std::vector<RTPSWriter*> automatic_writers_;
    //! List of the writers using manual by participant liveliness.
    std::vector<RTPSWriter*> manual_by_participant_writers_;
    //! List of writers using manual by topic liveliness
    std::vector<RTPSWriter*> manual_by_topic_writers_;

    //! List of readers
    std::vector<RTPSReader*> readers_;
    //! A boolean indicating that there is at least one reader requesting automatic liveliness
    bool automatic_readers_;

    //! A class used by writers in this participant to keep track of their liveliness
    LivelinessManager* pub_liveliness_manager_;
    //! A class used by readers in this participant to keep track of liveliness of matched writers
    LivelinessManager* sub_liveliness_manager_;

    /**
     * @brief A method invoked by pub_liveliness_manager_ to inform that a writer lost liveliness
     * @param writer The writer losing liveliness
     */
    void pub_liveliness_lost(GUID_t writer);

    /**
     * @brief A method invoked by pub_liveliness_manager_ to inform that a writer recovered liveliness
     * @param writer The writer recovering liveliness
     */
    void pub_liveliness_recovered(GUID_t writer);

    /**
     * @brief A method invoked by sub_liveliness_manager_ to inform that a writer lost liveliness
     * @param writer The writer losing liveliness
     * @param kind The liveliness kind of the writer losing liveliness
     * @param lease_duration The liveliness lease duration of the writer losing liveliness
     */
    void sub_liveliness_lost(
            const GUID_t& writer,
            const LivelinessQosPolicyKind& kind,
            const Duration_t& lease_duration);

    /**
     * @brief A method invoked by sub_liveliness_manager_ to inform that a writer recovered liveliness
     * @param writer The writer recovering liveliness
     * @param kind The liveliness kind of the writer recovering liveliness
     * @param lease_duration The liveliness lease duration of the writer recovering liveliness
     */
    void sub_liveliness_recovered(
            const GUID_t& writer,
            const LivelinessQosPolicyKind& kind,
            const Duration_t& lease_duration);


    /**
     * @brief A method to update the liveliness changed status of a given reader
     * @param writer The writer changing liveliness, specified by its guid
     * @param reader The reader whose liveliness needs to be updated
     * @param lost True to indicate that liveliness of the writer was lost. False to indicate it was recovered
     */
    void update_liveliness_changed_status(
            GUID_t writer,
            RTPSReader* reader,
            bool lost);

#if HAVE_SECURITY
    //!Pointer to the builtinRTPSParticipantMEssageWriter.
    StatefulWriter* mp_builtinWriterSecure;
    //!Pointer to the builtinRTPSParticipantMEssageReader.
    StatefulReader* mp_builtinReaderSecure;
    //!Writer History
    WriterHistory* mp_builtinWriterSecureHistory;
    //!Reader History
    ReaderHistory* mp_builtinReaderSecureHistory;

    /**
     * Create the secure endpoitns used in the WLP.
     * @return true if correct.
     */
    bool createSecureEndpoints();
#endif
};

}
} /* namespace rtps */
} /* namespace eprosima */
#endif
#endif /* WLP_H_ */
