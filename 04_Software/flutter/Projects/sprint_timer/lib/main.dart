import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:go_router/go_router.dart';

import 'core/theme/app_theme.dart';
import 'data/datasources/mock_datasource.dart';
import 'data/repositories/in_memory_repositories.dart';
import 'domain/entities/entities.dart';
import 'presentation/blocs/session_bloc.dart';
import 'presentation/blocs/live_session_bloc.dart';
import 'presentation/screens/screens_a.dart';
import 'presentation/screens/screens_b.dart';

void main() {
  final sessions = generateMockSessions();
  final sessionRepo = InMemorySessionRepository(sessions);
  final dataSource = MockSportDataSource();

  runApp(MyApp(
    sessionRepo: sessionRepo,
    dataSource: dataSource,
    sessions: sessions,
  ));
}

class MyApp extends StatelessWidget {
  final InMemorySessionRepository sessionRepo;
  final MockSportDataSource dataSource;
  final List<Session> sessions;

  MyApp({
    super.key,
    required this.sessionRepo,
    required this.dataSource,
    required this.sessions,
  });

  late final GoRouter router = GoRouter(
    routes: [
      GoRoute(
        path: '/',
        builder: (context, state) => const HomeScreen(),
      ),
      GoRoute(
        path: '/athletes',
        builder: (context, state) => const AthletesScreen(),
      ),
      GoRoute(
        path: '/history',
        builder: (context, state) => const HistoryScreen(),
      ),
      GoRoute(
        path: '/new-session',
        builder: (context, state) => const NewSessionScreen(),
      ),
      GoRoute(
        path: '/session/:id',
        builder: (context, state) {
          final id = state.pathParameters['id']!;
          return SessionDetailScreen(
            sessionId: id,
            allSessions: sessions,
          );
        },
      ),
      GoRoute(
        path: '/athlete/:id',
        builder: (context, state) {
          final id = int.parse(state.pathParameters['id']!);
          return AthleteDetailScreen(
            athleteId: id,
            allSessions: sessions,
          );
        },
      ),
      GoRoute(
        path: '/settings',
        builder: (context, state) => const SettingsScreen(),
      ),
      GoRoute(
        path: '/connection',
        builder: (context, state) => const ConnectionScreen(),
      ),
    ],
  );

  @override
  Widget build(BuildContext context) {
    return MultiBlocProvider(
      providers: [
        BlocProvider(
          create: (_) => SessionBloc(sessionRepo)..add(const LoadSessions()),
        ),
        BlocProvider(
          create: (_) => LiveSessionBloc(sessionRepo, dataSource),
        ),
      ],
      child: MaterialApp.router(
        title: 'Sprint Timer',
        debugShowCheckedModeBanner: false,
        theme: AppTheme.dark,
        routerConfig: router,
      ),
    );
  }
}
