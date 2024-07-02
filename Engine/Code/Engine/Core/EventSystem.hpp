#pragma once

#include "Engine/Core/NamedStrings.hpp"

#include <vector>
#include <string>
#include <map>
#include <mutex>

//----------------------------------------------------------------------------------------------------------------------
typedef NamedStrings EventArgs;
typedef bool (*EventCallbackFuncPtr)( EventArgs& eventArgs );
typedef std::vector<EventCallbackFuncPtr> EventSubscriberList;


// reference for learning purposes
// typedef std::vector<bool(*)(EventArgs&)> EventSubscriberList;
// std::function<EventArgs&e>
//----------------------------------------------------------------------------------------------------------------------
class EventSystem
{
public:
//	EventSystem( EventSystemConfig const& config );
	EventSystem();
	~EventSystem();
	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();

	void SubscribeToEvent	 ( std::string const& eventName, EventCallbackFuncPtr functionPtr );
	void UnsubscribeFromEvent( std::string const& eventName, EventCallbackFuncPtr functionPtr );
	void UnsubscribeFromAllEvents( EventCallbackFuncPtr callbackFunc );
	void FireEvent( std::string const& eventName, EventArgs& args );
	void FireEvent( std::string const& eventName );

	void GetNameOfRegisteredCommands( std::vector<std::string>& out_eventNames );

	template< typename T >
	void SubscribeEventCallbackObjectMethod( std::string const& eventName, T& object, bool (T::*method)( EventArgs& args ) );


	std::mutex m_eventSystemMutex;

private:
//	EventSystemConfig								m_config;
	std::map< std::string, EventSubscriberList >	m_subscribersForEventNames;
};

//----------------------------------------------------------------------------------------------------------------------
// Standalone global namespace help functions; these foward to "the" event system if it exists
void SubscribeToEvent	 ( std::string const& eventName, EventCallbackFuncPtr functionPtr );
void UnsubscribeFromEvent( std::string const& eventName, EventCallbackFuncPtr functionPtr );
void UnsubscribeFromAllEvents( EventCallbackFuncPtr callbackFunc );
void FireEvent( std::string const& eventName, EventArgs& args );
void FireEvent( std::string const& eventName );










// 
// //----------------------------------------------------------------------------------------------------------------------
// // 
// //----------------------------------------------------------------------------------------------------------------------
// typedef std::vector<EventSubscriptionBase> EventSubscriberList;
// 
// 
// //----------------------------------------------------------------------------------------------------------------------
// class EventSubscriptionBase
// {
// 	friend class EventSystem;
// 
// protected:
// 	virtual ~EventSubscriptionBase() = default;
// 	EventSubscriptionBase() = default;
// 	bool Execute( EventArgs& args ) = 0;
// 
// };
// 
// 
// //----------------------------------------------------------------------------------------------------------------------
// template< typename T >
// class EventSubscription_ObjectMethod : public EventSubscriptionBase
// {
// 	friend class EventSystem;
// 	typedef bool( T::*EventCallbackObjectMethod)( EventArgs& args );
// 
// protected:
// 	virtual ~EventSubscription_ObjectMethod() = default;
// 	EventSubscription_ObjectMethod( T& objectInstance, EventCallbackObjectMethod method )
// 		: m_object( objectInstance )
// 		, m_method( method )
// 	{
// 	}
// 	
// 	virtual bool Execute( EventArgs& args ) override;
// 	{
// 		bool wasConsumed = (m_object.*m_method)( args );
// 		return wasConsumed;
// 	}
// 
// protected:
// 	T& m_object;
// 	EventCallbackObjectMethod m_method = nullptr;
// };
// 
// 
// void SubscribeToEvent( std::string const& eventName, EventCallbackFuncPtr functionPtr )
// {
// 	// Create an entry
// 	SubscriptionList& subscribers = m_subscriptionListByEventName[ eventName ];
// 	// add to list of subscribers
// 	EventSubscription_ObjectMethod* newSubscriber = new EventSubscription_ObjectMethod( functionPtr );
// 	subscribers.push_back( newSubscriber );
// }
// 
// void FireEvent( std::string const& eventName, EventArgs& args )
// {
// 	// loop throuhg the eventList and find the event name
// 	if ( found == m_subscriptionListByEventName.end() )
// 	{
// 		return; // no subscribers found
// 	}
// 	// if found, get the value of all the subcscribers to this eent
// 	// loop through all eventListSubscribers and call Execute() on them, Execute() is a virtual function 
// 	// break loop if function was consumed
// }
// 
// 
// void UnSubscribeFromEvent( std::string const& eventName, EventCallbackFuncPtr functionPtr )
// {
// 	std::map< std::string, SubscriptionList >::iterator found = m_subscriptionListByEventName.find( eventName );
// 	if ( found == m_subscriptionListsByEventName.end() )
// 	{
// 		return;
// 	}
// 
// 	SubscriptionList& subscribersForThisEvent = found->second;
// 	for ( int i = 0; i< int( subscribersForThisEvent.size() ); i++ )
// 	{
// 		EventSubscriptionBase* subscriber = subscribersForThisEvent[i];
// 		if ( subscriber && subscriber->m_funcPtr == functionPtr )
// 		{
// 			delete subscriber;
// 			subscribersForThisEvent[i] = nullptr;
// 		}
// 	}
// }