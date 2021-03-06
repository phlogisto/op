/*
 
 Copyright (c) 2012, SMB Phone Inc.
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
 
 */

#pragma once

#include <hookflash/services/internal/hookflashTypes.h>
#include <hookflash/services/IRUDPICESocket.h>
#include <hookflash/services/IICESocket.h>

#include <map>

namespace hookflash
{
  namespace services
  {
    namespace internal
    {
      interaction IRUDPICESocketForRUDPICESocketSession
      {
        virtual IICESocketPtr getICESocket() const = 0;
        virtual IRUDPICESocketPtr getRUDPICESocket() const = 0;

        virtual zsLib::RecursiveLock &getLock() const = 0;

        virtual void onRUDPICESessionClosed(zsLib::PUID sessionID) = 0;
      };

      class RUDPICESocket : public zsLib::MessageQueueAssociator,
                            public IRUDPICESocket,
                            public IICESocketDelegate,
                            public IRUDPICESocketForRUDPICESocketSession
      {
      public:
        typedef zsLib::String String;

      protected:
        class Subscription;
        typedef boost::shared_ptr<Subscription> SubscriptionPtr;
        typedef boost::weak_ptr<Subscription> SubscriptionWeakPtr;

      protected:
        RUDPICESocket(
                      zsLib::IMessageQueuePtr queue,
                      IRUDPICESocketDelegatePtr delegate
                      );

        void init(
                  const char *turnServer,
                  const char *turnServerUsername,
                  const char *turnServerPassword,
                  const char *stunServer,
                  zsLib::WORD port = 0
                  );
        void init(
                  IDNS::SRVResultPtr srvTURNUDP,
                  IDNS::SRVResultPtr srvTURNTCP,
                  const char *turnServerUsername,
                  const char *turnServerPassword,
                  IDNS::SRVResultPtr srvSTUN,
                  zsLib::WORD port = 0
                  );

      public:
        ~RUDPICESocket();

        //IRUDPICESocket
        static RUDPICESocketPtr create(
                                       zsLib::IMessageQueuePtr queue,
                                       IRUDPICESocketDelegatePtr delegate,
                                       const char *turnServer,
                                       const char *turnServerUsername,
                                       const char *turnServerPassword,
                                       const char *stunServer,
                                       zsLib::WORD port = 0
                                       );

        static RUDPICESocketPtr create(
                                       zsLib::IMessageQueuePtr queue,
                                       IRUDPICESocketDelegatePtr delegate,
                                       IDNS::SRVResultPtr srvTURNUDP,
                                       IDNS::SRVResultPtr srvTURNTCP,
                                       const char *turnServerUsername,
                                       const char *turnServerPassword,
                                       IDNS::SRVResultPtr srvSTUN,
                                       zsLib::WORD port = 0
                                       );

        virtual zsLib::PUID getID() const {return mID;}

        virtual RUDPICESocketStates getState() const;

        virtual IRUDPICESocketSubscriptionPtr subscribe(IRUDPICESocketDelegatePtr delegate);

        virtual void shutdown();

        virtual void wakeup(zsLib::Duration minimumTimeCandidatesMustRemainValidWhileNotUsed);

        virtual void getLocalCandidates(CandidateList &outCandidates);

        virtual IRUDPICESocketSessionPtr createSessionFromRemoteCandidates(
                                                                           IRUDPICESocketSessionDelegatePtr delegate,
                                                                           const CandidateList &remoteCandidates,
                                                                           ICEControls control
                                                                           );

        //IICESocketDelegate
        virtual void onICESocketStateChanged(
                                             IICESocketPtr socket,
                                             ICESocketStates state
                                             );

        // IRUDPICESocketForRUDPICESocketSession
        virtual zsLib::RecursiveLock &getLock() const {return mLock;}

        virtual IICESocketPtr getICESocket() const;
        virtual IRUDPICESocketPtr getRUDPICESocket() const;

        virtual void onRUDPICESessionClosed(zsLib::PUID sessionID);

      protected:

        void cancelSubscription(zsLib::PUID subscriptionID);

      protected:
        String log(const char *message) const;
        bool isShuttingDown();
        bool isShutdown();

        void cancel();
        void setState(RUDPICESocketStates state);

      protected:
        class Subscription : public IRUDPICESocketSubscription
        {
        protected:
          Subscription(RUDPICESocketPtr outer);

        public:
          ~Subscription();

          static SubscriptionPtr create(RUDPICESocketPtr outer);

          virtual void cancel();

        public:
          RUDPICESocketWeakPtr mOuter;
          zsLib::PUID mID;
        };

      protected:
        mutable zsLib::RecursiveLock mLock;
        RUDPICESocketWeakPtr mThisWeak;
        RUDPICESocketPtr mGracefulShutdownReference;
        zsLib::PUID mID;

        RUDPICESocketStates mCurrentState;

        typedef std::map<zsLib::PUID, IRUDPICESocketDelegatePtr> DelegateMap;
        DelegateMap mDelegates;

        IICESocketPtr mICESocket;

        typedef std::map<zsLib::PUID, RUDPICESocketSessionPtr> SessionMap;
        SessionMap mSessions;
      };
    }
  }
}

ZS_DECLARE_PROXY_BEGIN(hookflash::services::internal::IRUDPICESocketForRUDPICESocketSession)
ZS_DECLARE_PROXY_METHOD_SYNC_CONST_RETURN_0(getLock, zsLib::RecursiveLock &)
ZS_DECLARE_PROXY_METHOD_SYNC_CONST_RETURN_0(getICESocket, hookflash::services::IICESocketPtr)
ZS_DECLARE_PROXY_METHOD_SYNC_CONST_RETURN_0(getRUDPICESocket, hookflash::services::IRUDPICESocketPtr)
ZS_DECLARE_PROXY_METHOD_1(onRUDPICESessionClosed, zsLib::PUID)
ZS_DECLARE_PROXY_END()
